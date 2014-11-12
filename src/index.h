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
#include <set>
#include <json/json.h>

#include "redis.h"
#include "md5.h"

#define IDX_SIZE 100000

// Index types (entities, relations, fields)
#define IDX_TYPE_ENT 0
#define IDX_TYPE_REL 1
#define IDX_TYPE_FIELD 2

// JSON Attribute Macros
#define JSON_ATTR_ENT_ENT "entity"
#define JSON_ATTR_ENT_FIELDS "fields"
#define JSON_ATTR_FIELDS_COUNT "fields_count"
#define JSON_ATTR_REL_ENTL "entity_left"
#define JSON_ATTR_REL_ENTR "entity_right"
#define JSON_ATTR_REL_FIELDSL "fields_left"
#define JSON_ATTR_REL_FIELDSR "fields_right"


using namespace std;

// TODO - implement heap UDT to handle

class IndexHandler {

    RedisHandler* redisHandler;

    void buildFieldJSONDefinition(Json::Value&, std::vector<std::pair<ColumnBase*, std::string>>*);
    void buildFieldJSONValue(Json::Value&, std::vector<std::pair<std::string, std::string>>*);

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() { this->redisHandler = new RedisHandler(REDISHOST, REDISPORT); }
    ~IndexHandler() { delete redisHandler; }

    bool writeEntity(std::string, vector<std::pair<ColumnBase*, std::string>>*);
    bool writeRelation(std::string, std::string, std::vector<std::pair<std::string, std::string>>*, std::vector<std::pair<std::string, std::string>>*);
    bool writeToDisk(int);

    Json::Value* composeJSON(std::string);
    Json::Value* fetchEntity(std::string);
    Json::Value* fetchRelation(std::string, std::string);

    bool existsEntity(std::string);
    bool existsRelation(std::string, std::string);

    std::vector<Json::Value>* fetchPatternJson(std::string);
    std::vector<std::string>* fetchPatternKeys(std::string);
    bool fetchFromDisk(int);   // Loads disk

    std::string generateEntityKey(std::string);
    std::string generateRelationKey(std::string, std::string);

    bool validateEntityFieldType(std::string, std::string, std::string);
    std::string orderPairAlphaNumeric(std::string, std::string);

};

/** Generate a key for an entity entry in the index */
std::string IndexHandler::generateEntityKey(std::string entity) { return "ent_" + entity; }

/** Generate a key for a relation entry in the index */
std::string IndexHandler::generateRelationKey(std::string entityL, std::string entityR, std::string hash) {
    return "rel_" + this->orderPairAlphaNumeric(entityL, entityR) + "_" + hash;
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONDefinition(Json::Value& value, std::vector<std::pair<ColumnBase*, std::string>>* fields) {
    int count = 0;
    for (std::vector<std::pair<ColumnBase*, std::string>>::iterator it = fields->begin() ; it != fields->end(); ++it) {
        value[(*it).second] = (*it).first->getType();
        count++;
    }
    value[JSON_ATTR_FIELDS_COUNT] = std::to_string(count);
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONValue(Json::Value& value, std::vector<std::pair<std::string, std::string>>* fields) {
    int count = 0;
    for (std::vector<std::pair<std::string, std::string>>::iterator it = fields->begin() ; it != fields->end(); ++it) {
        value[(*it).first] = (*it).second;
        count++;
    }
    value[JSON_ATTR_FIELDS_COUNT] = std::to_string(count);
}

/**
 * Writes of entities to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeEntity(std::string entity, std::vector<std::pair<ColumnBase*, std::string>>* fields) {
    Json::Value jsonVal;
    jsonVal[JSON_ATTR_ENT_ENT] = entity;
    this->buildFieldJSONDefinition(jsonVal, fields);
    jsonVal[JSON_ATTR_ENT_FIELDS] = jsonVal;
    this->redisHandler->connect();
    this->redisHandler->write(this->generateEntityKey(entity), jsonVal.toStyledString());
    return true;
}

/**
 * Writes relation to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeRelation(
                    std::string entityL,
                    std::string entityR,
                    std::vector<std::pair<std::string, std::string>>* fieldsL,
                    std::vector<std::pair<std::string, std::string>>* fieldsR) {
    Json::Value jsonVal;
    Json::Value jsonValFieldsLeft;
    Json::Value jsonValFieldsRight;

    jsonVal[JSON_ATTR_REL_ENTL] = entityL;
    jsonVal[JSON_ATTR_REL_ENTR] = entityR;

    this->buildFieldJSONValue(jsonValFieldsLeft, fieldsL);
    this->buildFieldJSONValue(jsonValFieldsRight, fieldsR);

    jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
    jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;

    this->redisHandler->connect();
    this->redisHandler->write(this->generateRelationKey(entityL, entityR, md5(jsonVal.asCString())), jsonVal.toStyledString());
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
Json::Value* IndexHandler::composeJSON(std::string key) {
    Json::Value* inMem = new Json::Value();
    Json::Reader reader;
    bool parsedSuccess;
    parsedSuccess = reader.parse(key, *inMem, false);
    if (parsedSuccess)
        return inMem;
    else
        return NULL;
}

/** Attempts to fetch an entity from index */
Json::Value* IndexHandler::fetchEntity(std::string entity) {
    this->redisHandler->connect();
    if (this->existsEntity(entity))
        return this->composeJSON(this->redisHandler->read(this->generateEntityKey(entity)));
    else
        return NULL;
}

/** Attempts to fetch a relation from index */
Json::Value* IndexHandler::fetchRelation(std::string entityL, std::string entityR) {
    this->redisHandler->connect();
    if (this->existsRelation(entityL, entityR))
        return this->composeJSON(this->redisHandler->read(this->generateRelationKey(entityL, entityR, "*")));
    else
        return NULL;
}

/** Check to ensure entity exists */
bool IndexHandler::existsEntity(std::string entity) {
    this->redisHandler->connect();
    return this->redisHandler->exists(this->generateEntityKey(entity));
}

/** Check to ensure relation exists */
bool IndexHandler::existsRelation(std::string entityL, std::string entityR) {
    this->redisHandler->connect();
    return this->redisHandler->exists(this->generateRelationKey(entityL, entityR, "*"));
}

/**
 * Fetch all redis records matching a pattern
 *
 * Returns a vector of JSON values
 */
std::vector<Json::Value>* IndexHandler::fetchPatternJson(std::string pattern) {
    std::vector<Json::Value>* elems = new std::vector<Json::Value>();
    Json::Value inMem;
    Json::Reader reader;
    bool parsedSuccess;
    std::vector<string>* vec;

    this->redisHandler->connect();
    vec = this->redisHandler->keys(pattern);

    // Iterate over all entries returned from redis
    for (std::vector<std::string>::iterator it = vec->begin() ; it != vec->end(); ++it) {
        parsedSuccess = reader.parse(this->redisHandler->read(*it), inMem, false);
        if (parsedSuccess)
            elems->push_back(inMem);
    }
    return elems;
}

/**
 * Fetch all redis keys matching a pattern
 *
 * Returns a vector of matching keys
 */
std::vector<string>* IndexHandler::fetchPatternKeys(std::string pattern) {
    std::vector<std::string>* elems = new std::vector<std::string>();
    std::vector<string>* vec;
    this->redisHandler->connect();
    vec = this->redisHandler->keys(pattern);
    for (std::vector<std::string>::iterator it = vec->begin() ; it != vec->end(); ++it)
        elems->push_back((*it).substr(4, (*it).length()));
    return elems;
}

/** Ensure that the field type is valid */
bool IndexHandler::validateEntityFieldType(std::string entity, std::string field, std::string value) {
    bool valid = true;
    Json::Value* jsonVal = this->fetchEntity(entity);
    valid = valid && jsonVal != NULL;
    if (valid) valid = valid && ((*jsonVal)[JSON_ATTR_ENT_FIELDS].isMember(field.c_str())); // ensure field exists
    if (valid) valid = valid && getColumnType((*jsonVal)[JSON_ATTR_ENT_FIELDS][field].asCString())->validate(value); // ensure the value is a valid instance of the type
    return valid;
}

/** Orders parameters alphanumerically then combines into one string */
std::string IndexHandler::orderPairAlphaNumeric(std::string s1, std::string s2) {
    std::set<std::string> sortedItems;
    std::set<std::string>::iterator it;
    std::string ret = "";
    sortedItems.insert(s1);
    sortedItems.insert(s2);
    it = sortedItems.begin(); ret = *it + "_"; it++; ret += *it;
    return ret;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
