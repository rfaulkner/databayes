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
#include "model.h"
#include <json/json.h>


using namespace std;

class Bayes {
    IndexHandler* indexHandler;

public:
    Bayes() { this->indexHandler = new IndexHandler(); }
    ~Bayes() { delete this->indexHandler; }

    float computeMarginal(std::string, AttributeBucket&);
    float computeConditional(std::string, std::string, AttributeBucket&);
    float computePairwise(std::string, std::string, AttributeBucket&);

    // Sample relations
    Relation sampleMarginal(std::string, AttributeBucket&);
    Relation sampleMarginal(Entity&, AttributeBucket&);
    Relation samplePairwise(std::string, std::string, AttributeBucket&);
    Relation samplePairwise(Entity&, Entity&, AttributeBucket&);
    Relation samplePairwiseCausal(std::string, std::string, AttributeBucket&);
    Relation samplePairwiseCausal(Entity&, Entity&, AttributeBucket&);

    // Compute expected values and mode for an attribute conditioned on filter values
    float expectedAttribute(AttributeTuple&, AttributeBucket&);
    std::string modeAttribute(AttributeTuple&, AttributeBucket&);

    long countEntityInRelations(std::string, AttributeBucket&, bool);
    long countRelations(std::string, std::string, AttributeBucket&);

};

/** Count the occurrences of a relation subject to a set of attribute filters */
long Bayes::countRelations(std::string e1, std::string e2, AttributeBucket& attrs) {
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(e1, e2);
    this->indexHandler->filterRelations(relations, attrs, ATTR_TUPLE_COMPARE_EQ);
    long total_relations = 0;
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) total_relations += (*it)[JSON_ATTR_REL_COUNT].asInt();
    return total_relations;
}

/** Count the occurrences of an entity among relevant relations */
long Bayes::countEntityInRelations(std::string e, AttributeBucket& attrs, bool causal=false) {
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e);

    // Filter on attribute conditions in AttributeBucket
    this->indexHandler->filterRelations(relations_left, attrs, ATTR_TUPLE_COMPARE_EQ);
    this->indexHandler->filterRelations(relations_right, attrs, ATTR_TUPLE_COMPARE_EQ);

    // Count the relations
    long total_relations = 0;
    for (std::vector<Json::Value>::iterator it = relations_left.begin(); it != relations_left.end(); ++it) {
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), e.c_str()) != 0 && causal) continue;
        total_relations += (*it)[JSON_ATTR_REL_COUNT].asInt();
    }

    for (std::vector<Json::Value>::iterator it = relations_right.begin(); it != relations_right.end(); ++it) {
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), e.c_str()) != 0 && causal) continue;
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
 *
 *  args:   the entity models & filter attributes
 **/
Relation Bayes::sampleMarginal(Entity& e, AttributeBucket& attrs) { return this->sampleMarginal(e.name, attrs); }

/*
 *  Samples an entity from the marginal distribution with respect to the filter attributes
 *
 *  args:   the entity names & filter attributes
 **/
Relation Bayes::sampleMarginal(std::string e, AttributeBucket& attrs) {

    // Find all relations with containing "e"
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e);

    // Filter on attribute conditions in AttributeBucket
    this->indexHandler->filterRelations(relations_left, attrs, ATTR_TUPLE_COMPARE_EQ);
    this->indexHandler->filterRelations(relations_right, attrs, ATTR_TUPLE_COMPARE_EQ);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countEntityInRelations(e, attrs);
    long index = 0;
    long pivot = rand() % count + 1;

    // Iterate through relations and pick out candidate
    for (std::vector<Json::Value>::iterator it = relations_left.begin(); it != relations_left.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            Relation r = Relation(*it);
            return r;
        }
    }

    for (std::vector<Json::Value>::iterator it = relations_right.begin(); it != relations_right.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            Relation r = Relation(*it);
            return r;
        }
    }

    return Relation(relations_right.back());
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 *
 *  args:   the entity models & filter attributes
 **/
Relation Bayes::samplePairwise(Entity& x, Entity& y, AttributeBucket& attrs) {
    return this->samplePairwise(x.name, y.name, attrs);
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 *
 *  args:   the entity names & filter attributes
 **/
Relation Bayes::samplePairwise(std::string x, std::string y, AttributeBucket& attrs) {

    // Find all relations with containing "x" and "y"
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(x, y);

    // Filter relations based on "attrs"
    this->indexHandler->filterRelations(relations, attrs, ATTR_TUPLE_COMPARE_EQ);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countRelations(x, y, attrs);
    long index = 0;
    long pivot = rand() % count + 1;

    // Iterate through relations and pick out candidate
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot)
            return Relation(*it);
    }

    return Relation(relations.back());
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 *
 *  args:   the entity models & filter attributes
 **/
Relation Bayes::samplePairwiseCausal(Entity& x, Entity& y, AttributeBucket& attrs) {
    return this->samplePairwiseCausal(x.name, y.name, attrs);
}

/*
 *  Samples an entity from the pairwise distribution with respect to the filter attributes
 *
 *  args:   the entity names & filter attributes
 **/
Relation Bayes::samplePairwiseCausal(std::string x, std::string y, AttributeBucket& attrs) {

    // Find all relations with containing "x" primary and "y" secondary
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(x, y);

    // Filter relations based on "attrs"
    this->indexHandler->filterRelations(relations, attrs, ATTR_TUPLE_COMPARE_EQ);

    // Randomly select a sample paying attention to frequency of relations
    long count = this->countEntityInRelations(x, attrs, true);
    long index = 0;
    long pivot = rand() % count + 1;

    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        Relation r = Relation(*it);

        // only consider elements in which x is the "cause"
        if (std::strcmp((*it)[JSON_ATTR_REL_CAUSE].asCString(), x.c_str()) != 0)
            continue;

        index += (*it)[JSON_ATTR_REL_COUNT].asInt();
        if (index >= pivot) {
            return r;
        }
    }
    return Relation(relations.back());
}

/**
 *  Produce the expected value for an attribute given a set of filter criteria
 *
 *  @param attr     the attribute to compute
 *  @param filter   filter criteria
 *
 *  @returns        The expected value for this attribute
 *                  TODO - handle error modes
 **/
float Bayes::expectedAttribute(AttributeTuple& attr, AttributeBucket& filter) {

    // Ensure that the attribute is a numeric type
    Json::Value json;
    this->indexHandler->fetchEntity(attr.entity, json);
    if (!json[JSON_ATTR_ENT_FIELDS].isMember(attr.attribute)) return -1.0;
    if (std::strcmp(json[JSON_ATTR_ENT_FIELDS][attr.attribute].asCString(), "integer") != 0 && std::strcmp(json[JSON_ATTR_ENT_FIELDS][attr.attribute].asCString(), "float") != 0) return -1.0;

    // Fetch all matching attributes
    std::vector<Json::Value> relations = this->indexHandler->fetchAttribute(attr);

    // Filter relations based on filter bucket
    this->indexHandler->filterRelations(relations, filter, ATTR_TUPLE_COMPARE_EQ);

    long count = 0;
    float expected = 0.0;

    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        if ((*it)[JSON_ATTR_REL_FIELDSL].isMember(attr.attribute))
            expected += (*it)[JSON_ATTR_REL_FIELDSL][attr.attribute].asFloat() * (*it)[JSON_ATTR_REL_COUNT].asFloat();
        if ((*it)[JSON_ATTR_REL_FIELDSR].isMember(attr.attribute))
            expected += (*it)[JSON_ATTR_REL_FIELDSL][attr.attribute].asFloat() * (*it)[JSON_ATTR_REL_COUNT].asFloat();
        count += (*it)[JSON_ATTR_REL_COUNT].asInt();
    }
    return expected / count;
}

/**
 *  Choose the value of an attribute corresponding to the maximum mode given a set of filter criteria
 *
 *  @param attr     the attribute to compute
 *  @param filter   filter criteria
 *
 *  @returns        The value representing the mode for this attribute
 *                  TODO - handle error modes
 **/
std::string Bayes::modeAttribute(AttributeTuple& attr, AttributeBucket& filter) {

    // Ensure that the attribute is present in the entity
    Json::Value json, counts;
    this->indexHandler->fetchEntity(attr.entity, json);
    if (!json[JSON_ATTR_ENT_FIELDS].isMember(attr.attribute)) return "";

    // Fetch all matching attributes
    std::vector<Json::Value> relations = this->indexHandler->fetchAttribute(attr);

    // Filter relations based on filter bucket
    this->indexHandler->filterRelations(relations, filter, ATTR_TUPLE_COMPARE_EQ);

    // Check left and right field sets; if the attribute is found count the instances for that value
    const char* key;
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        if ((*it)[JSON_ATTR_REL_FIELDSL].isMember(attr.attribute)) {
            key = (*it)[JSON_ATTR_REL_FIELDSL][attr.attribute].asCString();
            if (counts.isMember(key))
                counts[key] = counts[key].asInt() + (*it)[JSON_ATTR_REL_COUNT].asInt();
            else
                counts[key] = (*it)[JSON_ATTR_REL_COUNT].asInt();
        }
        if ((*it)[JSON_ATTR_REL_FIELDSR].isMember(attr.attribute)) {
            key = (*it)[JSON_ATTR_REL_FIELDSR][attr.attribute].asCString();
            if (counts.isMember(key))
                counts[key] = counts[key].asInt() + (*it)[JSON_ATTR_REL_COUNT].asInt();
            else
                counts[key] = (*it)[JSON_ATTR_REL_COUNT].asInt();
        }
    }

    // Get the key with the most occurrences - the key is across the range of values for the attribute
    std::vector<std::string> keys = counts.getMemberNames();
    int max = 0;
    std:string value;
    for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); ++it) {
        if (counts[*it].asInt() > max) {
            max = counts[*it].asInt();
            value = *it;
        }
    }
    return value;
}

#endif
