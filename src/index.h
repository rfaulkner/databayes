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

#define KEY_DELIMETER "+"

// JSON Attribute Macros
#define JSON_ATTR_ENT_ENT "entity"
#define JSON_ATTR_ENT_FIELDS "fields"
#define JSON_ATTR_FIELDS_COUNT "_itemcount"
#define JSON_ATTR_REL_COUNT "instance_count"
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

    void removeEntity(std::string);
    void removeRelation(std::string, std::string, std::vector<std::pair<std::string, std::string>>*, std::vector<std::pair<std::string, std::string>>*);

    bool composeJSON(std::string, Json::Value&);
    bool fetchRaw(std::string, Json::Value&);
    bool fetchEntity(std::string, Json::Value&);
    std::vector<Json::Value> fetchRelationPrefix(std::string, std::string);

    bool existsEntity(std::string);
    bool existsEntityField(std::string, std::string);
    bool existsRelation(std::string, std::string);

    std::vector<Json::Value>* fetchPatternJson(std::string);
    std::vector<std::string>* fetchPatternKeys(std::string);
    bool fetchFromDisk(int);   // Loads disk

    std::string generateEntityKey(std::string);
    std::string generateRelationKey(std::string, std::string, std::string);

    bool validateEntityFieldType(std::string, std::string, std::string);
    std::string orderPairAlphaNumeric(std::string, std::string);

};

/** Generate a key for an entity entry in the index */
std::string IndexHandler::generateEntityKey(std::string entity) {
    std::string ent("ent");
    std::string delim(KEY_DELIMETER);
    return ent + delim + entity;
}

/** Generate a key for a relation entry in the index */
std::string IndexHandler::generateRelationKey(std::string entityL, std::string entityR, std::string hash) {
    std::string rel("rel");
    std::string delim(KEY_DELIMETER);
    return rel + delim + this->orderPairAlphaNumeric(entityL, entityR) + delim + hash;
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONDefinition(Json::Value& value, std::vector<std::pair<ColumnBase*, std::string>>* fields) {
    int count = 0;
    for (std::vector<std::pair<ColumnBase*, std::string>>::iterator it = fields->begin() ; it != fields->end(); ++it) {
        value[(*it).second] = (*it).first->getType();
        count++;
    }
    value[JSON_ATTR_FIELDS_COUNT] = count;
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONValue(Json::Value& value, std::vector<std::pair<std::string, std::string>>* fields) {
    int count = 0;
    for (std::vector<std::pair<std::string, std::string>>::iterator it = fields->begin() ; it != fields->end(); ++it) {
        value[(*it).first] = (*it).second;
        count++;
    }
    value[JSON_ATTR_FIELDS_COUNT] = count;
}

/**
 * Writes of entities to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeEntity(std::string entity, std::vector<std::pair<ColumnBase*, std::string>>* fields) {
    Json::Value jsonVal;
    Json::Value jsonValFields;
    jsonVal[JSON_ATTR_ENT_ENT] = entity;
    this->buildFieldJSONDefinition(jsonValFields, fields);
    jsonVal[JSON_ATTR_ENT_FIELDS] = jsonValFields;
    this->redisHandler->connect();
    this->redisHandler->write(this->generateEntityKey(entity), jsonVal.toStyledString());
    return true;
}

/** Remove entity key from redis */
void IndexHandler::removeEntity(std::string entity) {
    this->redisHandler->connect();
    this->redisHandler->deleteKey(this->generateEntityKey(entity));
    // TODO - ensure that relations are consistent
}

/** Remove relation from redis */
void IndexHandler::removeRelation(
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
    this->redisHandler->deleteKey(this->generateRelationKey(entityL, entityR, md5(jsonVal.toStyledString())));
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
    std::string key;
    Json::Value jsonValFieldsLeft;
    Json::Value jsonValFieldsRight;

    jsonVal[JSON_ATTR_REL_ENTL] = entityL;
    jsonVal[JSON_ATTR_REL_ENTR] = entityR;

    this->buildFieldJSONValue(jsonValFieldsLeft, fieldsL);
    this->buildFieldJSONValue(jsonValFieldsRight, fieldsR);

    jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
    jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;

    this->redisHandler->connect();
    key = this->generateRelationKey(entityL, entityR, md5(jsonVal.toStyledString()));

    if (this->redisHandler->exists(key)) {
        if (this->fetchRaw(key, jsonVal)) {
            jsonVal[JSON_ATTR_REL_COUNT] = jsonVal[JSON_ATTR_REL_COUNT].asInt() + 1;
        } else
            return false;
    } else {jsonVal[JSON_ATTR_REL_COUNT] = 1;}


    this->redisHandler->write(key, jsonVal.toStyledString());
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
bool IndexHandler::composeJSON(std::string key, Json::Value& json) {
    Json::Reader reader;
    bool parsedSuccess;
    parsedSuccess = reader.parse(key, json, false);
    if (parsedSuccess)
        return true;
    else
        return false;
}

/** Attempts to fetch an entity from index */
bool IndexHandler::fetchEntity(std::string entity, Json::Value& json) {
    this->redisHandler->connect();
    if (this->existsEntity(entity)) {
        if (this->composeJSON(this->redisHandler->read(this->generateEntityKey(entity)), json))
            return true;
        else
            return false;
    } else
        return false;
}

/** Attempts to fetch a key from index */
bool IndexHandler::fetchRaw(std::string key, Json::Value& json) {
    this->redisHandler->connect();
    if (this->redisHandler->exists(key)) {
        if (this->composeJSON(this->redisHandler->read(key), json))
            return true;
        else
            return false;
    } else
        return false;
}

/** Fetch a set of relations matching the entities */
std::vector<Json::Value> IndexHandler::fetchRelationPrefix(std::string entityL, std::string entityR) {
    std::vector<std::string> keys = this->redisHandler->keys(this->generateRelationKey(entityL, entityR, "*"));
    std::vector<Json::Value> relations;
    Json::Value* json;
    for (std::vector<string>::iterator it = keys.begin(); it != keys.end(); ++it) {
        json = new Json::Value();
        this->composeJSON(this->redisHandler->read(*it), *json);
        relations.push_back(*json);
    }
    return relations;
}

/** Check to ensure entity exists */
bool IndexHandler::existsEntity(std::string entity) {
    this->redisHandler->connect();
    return this->redisHandler->exists(this->generateEntityKey(entity));
}

/** Check to ensure entity exists */
bool IndexHandler::existsEntityField(std::string entity, std::string field) {
    Json::Value json;
    this->fetchEntity(entity, json);
    return json.isMember(field);
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
    std::vector<string> vec;

    this->redisHandler->connect();
    vec = this->redisHandler->keys(pattern);

    // Iterate over all entries returned from redis
    for (std::vector<std::string>::iterator it = vec.begin() ; it != vec.end(); ++it) {
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
    std::vector<string> vec;
    this->redisHandler->connect();
    vec = this->redisHandler->keys(pattern);
    for (std::vector<std::string>::iterator it = vec.begin() ; it != vec.end(); ++it)
        elems->push_back((*it).substr(4, (*it).length()));
    return elems;
}

/** Ensure that the field type is valid */
bool IndexHandler::validateEntityFieldType(std::string entity, std::string field, std::string value) {
    bool valid = true;
    Json::Value json;
    valid = valid && this->fetchEntity(entity, json);
    if (valid) valid = valid && (json[JSON_ATTR_ENT_FIELDS].isMember(field.c_str())); // ensure field exists
    if (valid) valid = valid && getColumnType(json[JSON_ATTR_ENT_FIELDS][field].asCString())->validate(value); // ensure the value is a valid instance of the type
    return valid;
}

/** Orders parameters alphanumerically then combines into one string */
std::string IndexHandler::orderPairAlphaNumeric(std::string s1, std::string s2) {
    std::set<std::string> sortedItems;
    std::set<std::string>::iterator it;
    std::string ret = "";
    sortedItems.insert(s1);
    sortedItems.insert(s2);
    it = sortedItems.begin(); ret = *it + KEY_DELIMETER; it++; ret += *it;
    return ret;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
