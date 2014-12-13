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

    long countEntityInRelations(Entity&, std::vector<std::string, std::string>&);

public:
    Bayes() { this->indexHandler = new IndexHandler(); }

    float computeMarginal(Entity&, std::vector<std::string, std::string>&);
    float computeConditional(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);
    float computePairwise(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);

    Json::Value sampleMarginal(std::string, std::vector<std::string, std::string>&);
    Json::Value sampleConditional(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);
    Json::Value samplePairwise(std::string, std::string, std::vector<std::string, std::string>&, std::vector<std::string, std::string>&);

};

/** Count the occurrences of an entity among relevant relations */
long Bayes::countEntityInRelations(Entity& e, std::vector<std::string, std::string>& attrs) {
    std::vector<Json::Value> relations_left = this->indexHandler->fetchRelationPrefix(e.name, "*");
    std::vector<Json::Value> relations_right = this->indexHandler->fetchRelationPrefix("*", e.name);

    return this->indexHandler->filterRelationsByAttribute(relations_left, attrs).size() + this->indexHandler->filterRelationsByAttribute(relations_right, attrs).size();
}

/** Marginal probability of an entities determined by occurrences present in relations */
float Bayes::computeMarginal(Entity& e, std::vector<std::string, std::string>& attrs) {
    long total = this->indexHandler->getRelationCountTotal();
    if (total > 0)
        return (float)this->countEntityInRelations(e, attrs) / (float)total;
    else {
        cout << "DEBUG -- Bad total relation count: " << total << endl;
        return 0;
    }
}

#endif
