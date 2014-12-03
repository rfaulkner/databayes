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

    long countEntityInRelations(std::string, std::vector<std::string, std::string>&);

public:
    Bayes() { this->indexHandler = new IndexHandler(); }

    float computeMarginal(std::string, std::vector<std::string, std::string>&);
    float computeConditional(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);
    float computePairwise(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);

    Json::Value sampleMarginal(std::string, std::vector<std::string, std::string>&);
    Json::Value sampleConditional(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);
    Json::Value samplePairwise(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);

};

/** Count the occurrences of an entity among relevant relations */
long Bayes::countEntityInRelations(std::string entity, std::vector<std::string, std::string>& fields) {
    return this->indexHandler->filterRelationsByAttribute(this->indexHandler->generateRelationKey(entity, "*"), fields).size() +
            this->indexHandler->filterRelationsByAttribute(this->indexHandler->generateRelationKey("*", entity), fields).size();
}

#endif
