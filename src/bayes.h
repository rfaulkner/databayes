/*
 *  bayes.h
 *
 *  Defines the algorithmic class Bayes for computing probabilities distributed across relations and for generating samples
 *  relative to those distributions.
 *
 *  Created by Ryan Faulkner on 2014-11-29
 *  Copyright (c) 2014. All rights reserved.
 */


#ifndef _bayes_h
#define _bayes_h

#include <string>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "index.h"
#include <json/json.h>


using namespace std;

class Bayes {
    IndexHandler* indexHandler;

public:
    Bayes() { this->indexHandler = new IndexHandler(); }

    float computeMarginal(std::string, AttributeBucket&);
    float computeConditional(std::string, std::string, AttributeBucket&);
    float computePairwise(std::string, std::string, AttributeBucket&);

    Relation sampleMarginal(Entity&, AttributeBucket&);
    Relation samplePairwise(Entity&, Entity&, AttributeBucket&);
    Relation samplePairwiseCausal(Entity&, Entity&, AttributeBucket&);

    long countEntityInRelations(std::string, AttributeBucket&, bool);
    long countRelations(std::string, std::string, AttributeBucket&);

};

/** Count the occurrences of a relation subject to a set of attribute filters */
long Bayes::countRelations(std::string e1, std::string e2, AttributeBucket& attrs) {
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(e1, e2);
    relations = this->indexHandler->filterRelations(relations, attrs);
    long total_relations = 0;
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) total_relations += (*it)[JSON_ATTR_REL_COUNT].asInt();
    return total_relations;
}

/** Count the occurrences of an entity among relevant relations */
long Bayes::countEntityInRelations(std::string e, AttributeBucket& attrs, bool causal=false) {
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e);

    // Filter on attribute conditions in AttributeBucket
    relations_left = this->indexHandler->filterRelations(relations_left, attrs);
    relations_right = this->indexHandler->filterRelations(relations_right, attrs);

    // Count the relations
    long total_relations = 0;
    for (std::vector<Json::Value>::iterator it = relations_left.begin(); it != relations_left.end(); ++it) {
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), e.c_str()) != 0) continue;
        total_relations += (*it)[JSON_ATTR_REL_COUNT].asInt();
    }

    for (std::vector<Json::Value>::iterator it = relations_right.begin(); it != relations_right.end(); ++it) {
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), e.c_str()) != 0) continue;
        total_relations+= (*it)[JSON_ATTR_REL_COUNT].asInt();
    }

    return total_relations;
}

/** Marginal probability of an entities determined by occurrences present in relations */
float Bayes::computeMarginal(std::string e, AttributeBucket& attrs) {
    long total = this->indexHandler->getRelationCountTotal();
    if (total > 0)
        return (float)this->countEntityInRelations(e, attrs) / (float)total;
    else {
        cout << "DEBUG -- Bad total relation count: " << total << endl;
        return 0;
    }
}

/** Relation probabilities determined by occurrences present in relations */
float Bayes::computePairwise(std::string e1, std::string e2, AttributeBucket& attrs) {
    long total = this->indexHandler->getRelationCountTotal();
    if (total > 0)
        return (float)this->countRelations(e1, e2, attrs) / (float)total;
    else {
        cout << "DEBUG -- Bad total relation count: " << total << endl;
        return 0;
    }
}

/** Conditional Probabilities among entities */
float Bayes::computeConditional(std::string e1, std::string e2, AttributeBucket& attrs) {
    float pairwise = this->computePairwise(e1, e2, attrs);
    float marginal = this->computeMarginal(e2, attrs);

    if (marginal > 0)
        return pairwise / marginal;
    else {
        cout << "DEBUG -- marginal likelihood is 0" << endl;
        return 0;
    }
}

/*
 *  Samples an entity from the marginal distribution with respect to the filter attributes
 **/
Relation Bayes::sampleMarginal(Entity& e, AttributeBucket& attrs) {
    Relation r;

    // Find all relations with containing "e"
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e.name, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e.name);

    // Filter on attribute conditions in AttributeBucket
    relations_left = this->indexHandler->filterRelations(relations_left, attrs);
    relations_right = this->indexHandler->filterRelations(relations_right, attrs);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countEntityInRelations(e.name, attrs);
    long index = 0;
    long pivot = rand() % count + 1;

    // Iterate through relations and pick out candidate
    for (std::vector<Json::Value>::iterator it = relations_left.begin(); it != relations_left.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            r.fromJSON(*it);
            return r;
        }
    }

    for (std::vector<Json::Value>::iterator it = relations_right.begin(); it != relations_right.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            r.fromJSON(*it);
            return r;
        }
    }

    r.fromJSON(relations_right.back());
    return r;
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 **/
Relation Bayes::samplePairwise(Entity& x, Entity& y, AttributeBucket& attrs) {
    Relation r;

    // Find all relations with containing "x" and "y"
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(x.name, y.name);

    // Filter relations based on "attrs"
    relations = this->indexHandler->filterRelations(relations, attrs);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countRelations(x.name, y.name, attrs);
    long index = 0;
    long pivot = rand() % count + 1;

    // Iterate through relations and pick out candidate
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            r.fromJSON(*it);
            return r;
        }
    }

    r.fromJSON(relations.back());
    return r;
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 *
 **/
Relation Bayes::samplePairwiseCausal(Entity& x, Entity& y, AttributeBucket& attrs) {
    Relation r;

    // Find all relations with containing "x" primary and "y" secondary
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(x.name, y.name);

    // Filter relations based on "attrs"
    relations = this->indexHandler->filterRelations(relations, attrs);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countEntityInRelations(x.name, attrs, true);
    long index = 0;
    long pivot = rand() % count + 1;

    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {

        // only consider elements in which x is the "cause"
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), x.name.c_str()) != 0)
            continue;

        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        r.fromJSON(*it);
        if (index >= pivot) {
            return r;
        }
    }
    return r;
}

#endif
