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
#include "index.h"
#include <json/json.h>


using namespace std;

class Bayes {
    IndexHandler* indexHandler;

    long countEntityInRelations(Entity&, valpair&);
    long countRelations(std::string, std::string, valpair&);

public:
    Bayes() { this->indexHandler = new IndexHandler(); }

    float computeMarginal(Entity&, valpair&);
    float computeConditional(std::string, std::string, valpair&);
    float computePairwise(std::string, std::string, valpair&);

    Json::Value sampleMarginal(std::string, valpair&);
    Json::Value sampleConditional(std::string, std::string, valpair&);
    Json::Value samplePairwise(std::string, std::string, valpair&);

};

/** Count the occurrences of a relation */
long Bayes::countRelations(std::string e1, std::string e2, valpair& attrs) {
    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(e1.name, e2.name);
    return this->indexHandler->filterRelationsByAttribute(relations, attrs).size();
}

/** Count the occurrences of an entity among relevant relations */
long Bayes::countEntityInRelations(Entity& e, valpair& attrs) {
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e.name, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e.name);

    return this->indexHandler->filterRelationsByAttribute(relations_left, attrs).size() + this->indexHandler->filterRelationsByAttribute(relations_right, attrs).size();
}

/** Marginal probability of an entities determined by occurrences present in relations */
float Bayes::computeMarginal(Entity& e, valpair& attrs) {
    long total = this->indexHandler->getRelationCountTotal();
    if (total > 0)
        return (float)this->countEntityInRelations(e, attrs) / (float)total;
    else {
        cout << "DEBUG -- Bad total relation count: " << total << endl;
        return 0;
    }
}

/** Relation probabilities determined by occurrences present in relations */
float Bayes::computePairwise(std::string e1, std::string e2, valpair& attrs) {
    long total = this->indexHandler->getRelationCountTotal();
    if (total > 0)
        return (float)this->countRelations(e1, e2, attrs) / (float)total;
    else {
        cout << "DEBUG -- Bad total relation count: " << total << endl;
        return 0;
    }
}

/** Conditional Probabilities among entities */
float Bayes::computeConditional(std::string e1, std::string e2, valpair& attrs) {
    float pairwise = this->computePairwise(e1, e2, attrs);
    float marginal = this->computeMarginal(e2, attrs);

    if (marginal > 0)
        return pairwise / marginal;
    else {
        cout << "DEBUG -- marginal likelihood is 0" << endl;
        return 0;
    }
}

Json::Value sampleMarginal(std::string e, valpair& attrs) {
    return null;
}

Json::Value sampleConditional(std::string e1, std::string e2, valpair& attrs) {
    return null;
}

Json::Value samplePairwise(std::string e1, std::string e2, valpair& attrs) {
    return null;
}

#endif
