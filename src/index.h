/*
 *  index.h
 *
 *  Defines interface for the index.  Provides an in memory instance to fetch results quickly and
 *  provides an interface to disk storage where needed.  Use a heap to maintain order.
 *
 *  Created by Ryan Faulkner on 2014-09-20
 *
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _index_h
#define _index_h

#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>

#define IDX_SIZE 100000

// Index types (entities, relations, fields)
#define IDX_TYPE_ENT 0
#define IDX_TYPE_REL 1
#define IDX_TYPE_FIELD 2


using namespace std;

// TODO - implement heap UDT to handle

class IndexHandler {

    Json::Value* inMemEnt;  // In memory instance for entities
    Json::Value* inMemRel;  // In memory instance for relations

    int currEnt;
    int currRel;
    int* size;

    int diskBlock;

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() {
        this->inMemEnt = new string [IDX_SIZE];
        this->inMemRel = new string [IDX_SIZE];
        this->currEnt = 0;
        this->currRel = 0;

        this->size = new int [2];
        this->size[IDX_TYPE_ENT] = 0;
        this->size[IDX_TYPE_REL] = 0;
    }
    ~IndexHandler() { delete [] inMemEnt; delete [] inMemRel; }

    bool writeEntity(std::string, vector<std::pair<ColumnBase&, std::string>>);
    bool writeRelation(std::string, std::string, vector<std::pair<ColumnBase&, std::string>>, vector<std::pair<ColumnBase&, std::string>>);
    bool writeToDisk(int);

    bool fetch(int const, Json::Value&);
    Json::Value& fetchAll(int const);
    bool fetchFromDisk(int);   // Loads disk

    /**
     *  Getter for index size
     */
    int idxSize(int type) {
        if (type != IDX_TYPE_ENT && type != IDX_TYPE_REL)
            return 0;
        return this->size[type];
    }
};


/**
 * Writes of entities to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeEntity(string entity, std::vector<std::pair<ColumnBase&, std::string>> fields) {
    // TODO - Maintain sort order, heap

    Json::Value jsonVal;

    jsonVal["entity"] = entity;
    jsonVal["fields"] = &fields[0];

    this->inMemEnt[this->currEnt++] = &jsonVal;
    this->size[IDX_TYPE_ENT]++;

    return true;
}

/**
 * Writes relation to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeRelation(
                    string entityL,
                    string entityL,
                    std::vector<std::pair<ColumnBase&, std::string>> fieldsL,
                    std::vector<std::pair<ColumnBase&, std::string>> fieldsR) {
    // TODO - Maintain sort order, heap

    Json::Value jsonVal;

    jsonVal["entityL"] = entityL;
    jsonVal["fieldsL"] = &fieldsL[0];
    jsonVal["entityR"] = entityR;
    jsonVal["fieldsR"] = &fieldsR[0];

    this->inMemRel[this->currRel++] = &jsonVal;
    this->size[IDX_TYPE_REL]++;

    return true;
}

/**
 * Handles writes to disk with strategy
 *
 *  TODO - currently null
 */
bool IndexHandler::writeToDisk(int type) { return false; }

/**
 * Attempts to fetch an entry from index
 *
 *  TODO - this is currently incredibly inefficient, improve
 */
bool IndexHandler::fetch(int const type, Json::Value& entry) {

    string* inMem;
    int curr;

    // Determine the index type
    if (type == IDX_TYPE_ENT) {
        inMem = this->inMemEnt;
        curr = this->currEnt;
    } else if (type == IDX_TYPE_REL) {
        inMem = this->inMemRel;
        curr = this->currRel;
    } else // bad type
        return false;

    // Find the entry
    for (int i = 0; i < curr; i++)
        if (entry == this->inMemEnt[i])
            return true;
    return false;
}

/**
 * Fetch all entities
 *
 *  TODO - this only returns the in-memory index
 */
string* IndexHandler::fetchAll(int const type) {

    // Determine the index type
    if (type == IDX_TYPE_ENT) {
        return this->inMemEnt;
    } else if (type == IDX_TYPE_REL) {
        return this->inMemRel;
    } else // bad type
        return NULL;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
