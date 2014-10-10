/*
 *  index.h
 *
 *  THIS OPERATES IN MEMORY ONLY - NO DISK WRITES YET
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

#include "redis.h"

#define IDX_SIZE 100000

// Index types (entities, relations, fields)
#define IDX_TYPE_ENT 0
#define IDX_TYPE_REL 1
#define IDX_TYPE_FIELD 2


using namespace std;

// TODO - implement heap UDT to handle

class IndexHandler {

    RedisHandler* redisHandler;

    void buildFieldJSON(Json::Value&, std::vector<std::pair<ColumnBase*, std::string>>*, std::string);

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() { this->redisHandler = new RedisHandler(REDISHOST, REDISDB); }
    ~IndexHandler() { delete redisHandler; }

    bool writeEntity(std::string, vector<std::pair<ColumnBase*, std::string>>*);
    bool writeRelation(std::string, std::string, std::vector<std::pair<ColumnBase*, std::string>>*, std::vector<std::pair<ColumnBase*, std::string>>*);
    bool writeToDisk(int);

    Json::Value* fetch(int const, std::string);
    std::vector<Json::Value>* IndexHandler::fetchPattern(std::string);
    bool fetchFromDisk(int);   // Loads disk

};


/**
 * Handles forming the json for field vectors in the index
 */
private void IndexHandler::buildFieldJSON(Json::Value& value,
                                          std::vector<std::pair<ColumnBase*, std::string>>* fields,
                                          std::string prefix) {
    int counter = 0;
    for (std::vector<std::string>::iterator it = fieldsL->begin() ; it != fieldsL->end(); ++it) {
        jsonVal[prefix + "_type_" + std::to_string(i)] = *it[0]->getType();
        jsonVal[prefix + "_value_" + std::to_string(i)] = *it[1];
        counter++;
    }
}

/**
 * Writes of entities to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeEntity(
                        std::string key,
                        std::string entity,
                        std::vector<std::pair<ColumnBase*, std::string>>* fields) {
    Json::Value jsonVal;
    jsonVal["entity"] = entity;
    this->buildFieldJSON(&jsonVal, fields, "fields");
    this->RedisHandler->write(key, jsonVal.asString());
    return true;
}

/**
 * Writes relation to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeRelation(
                    std::string key,
                    std::string entityL,
                    std::string entityR,
                    std::vector<std::pair<ColumnBase*, std::string>>* fieldsL,
                    std::vector<std::pair<ColumnBase*, std::string>>* fieldsR) {
    Json::Value jsonVal;
    jsonVal["entityL"] = entityL;
    jsonVal["entityR"] = entityR;
    this->buildFieldJSON(&jsonVal, fieldsL, "fieldsL");
    this->buildFieldJSON(&jsonVal, fieldsR, "fieldsR");
    this->RedisHandler->write(key, jsonVal.asString());
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
 * @param string key    key string for the requested entry
 */
Json::Value* IndexHandler::fetch(std::string key) {
    Json::Value inMem;
    Json::Reader reader;
    this->RedisHandler->read(key);
    parsedSuccess = reader.parse(this->RedisHandler->read(key), inMem, false);
    if (parsedSuccess)
        return &inMem;
    else
        return NULL;
}

/**
 * Fetch all redis records matching a pattern
 *
 * Returns a vector of JSON values
 */
std::vector<Json::Value>* IndexHandler::fetchPattern(std::string pattern) {

    std::vector<Json::Value>* elems = new std::vector<Json::Value>();
    Json::Value inMem;
    Json::Reader reader;

    std::string redisRsp;
    std::vector<string>* vec;

    vec = this->RedisHandler.keys(pattern);

    // Iterate over all entries returned from redis
    for (std::vector<std::string>::iterator it = vec->begin() ; it != vec->end(); ++it) {
        parsedSuccess = reader.parse(*it, inMem, false);
        if (parsedSuccess)
            elems.push_back(inMem);
    }
    return elems;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
