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
#include <boost/regex.hpp>

#include "redis.h"
#include "md5.h"
#include "model.h"

#define IDX_SIZE 100000

// Index types (entities, relations, fields)
#define IDX_TYPE_ENT 0
#define IDX_TYPE_REL 1
#define IDX_TYPE_FIELD 2

#define KEY_DELIMETER "+"
#define KEY_TOTAL_RELATIONS "total_relations"

class IndexHandler {

    RedisHandler* redisHandler;

    void buildFieldJSONDefinition(Json::Value&, defpair&);
    void buildFieldJSONValue(Json::Value&, valpair&);

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() { this->redisHandler = new RedisHandler(REDISHOST, REDISPORT); }
    ~IndexHandler() { delete redisHandler; }

    bool writeEntity(Entity&);
    bool writeRelation(Relation&);
    bool writeToDisk(int);

    bool removeEntity(std::string);
    bool removeRelation(Relation&);
    bool removeRelation(Json::Value&);

    bool composeJSON(std::string, Json::Value&);

    bool existsEntity(std::string);
    bool existsEntityField(std::string, std::string);
    bool existsRelation(std::string, std::string);

    bool fetchRaw(std::string, Json::Value&);
    bool fetchEntity(std::string, Json::Value&);
    std::vector<Json::Value> fetchRelationPrefix(std::string, std::string);
    std::vector<Json::Value>* fetchPatternJson(std::string);
    std::vector<std::string>* fetchPatternKeys(std::string);
    bool fetchFromDisk(int);   // Loads disk

    std::vector<Relation> Json2RelationVector(std::vector<Json::Value>);
    std::vector<Json::Value> Relation2JsonVector(std::vector<Relation>);

    std::vector<Relation> filterRelations(std::vector<Relation>&, AttributeBucket&);
    std::vector<Json::Value> filterRelations(std::vector<Json::Value>&, AttributeBucket&);

    std::string generateEntityKey(std::string);
    std::string generateRelationKey(std::string, std::string, std::string);

    bool validateEntityFieldType(std::string, std::string, std::string);
    std::string orderPairAlphaNumeric(std::string, std::string);

    long getRelationCountTotal();
    void setRelationCountTotal(long value);

    long computeRelationsCount(std::string, std::string);
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
void IndexHandler::buildFieldJSONDefinition(Json::Value& value, defpair& fields) {
    int count = 0;
    for (defpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
        value[(*it).second] = (*it).first->getType();
        count++;
    }
    value[JSON_ATTR_FIELDS_COUNT] = count;
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONValue(Json::Value& value, valpair& fields) {
    int count = 0;
    for (valpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
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
bool IndexHandler::writeEntity(Entity& e) {
    Json::Value jsonVal;
    Json::Value jsonValFields;
    jsonVal[JSON_ATTR_ENT_ENT] = e.name;
    this->buildFieldJSONDefinition(jsonValFields, e.attrs);
    jsonVal[JSON_ATTR_ENT_FIELDS] = jsonValFields;
    this->redisHandler->connect();
    this->redisHandler->write(this->generateEntityKey(e.name), jsonVal.toStyledString());
    return true;
}

/** Remove entity key from redis */
bool IndexHandler::removeEntity(std::string entity) {
    std::string codedEntity = this->generateEntityKey(entity);
    this->redisHandler->connect();
    if (this->redisHandler->exists(codedEntity)) {
        this->redisHandler->deleteKey(codedEntity);

        // Delete all relations containing this entity
        std::vector<Json::Value> relations_left = this->fetchRelationPrefix(entity, "*");
        std::vector<Json::Value> relations_right = this->fetchRelationPrefix("*", entity);
        for (std::vector<Json::Value>::iterator it = relations_left.begin() ; it != relations_left.end(); ++it) this->removeRelation(*it);
        for (std::vector<Json::Value>::iterator it = relations_right.begin() ; it != relations_right.end(); ++it) this->removeRelation(*it);

        return true;
    }
    return false;
}

/** Wraps removeRelation(Json::Value&) */
bool IndexHandler::removeRelation(Relation& rel) {

    Json::Value jsonVal;
    Json::Value jsonValFieldsLeft;
    Json::Value jsonValFieldsRight;

    jsonVal[JSON_ATTR_REL_ENTL] = rel.name_left;
    jsonVal[JSON_ATTR_REL_ENTR] = rel.name_right;
    this->buildFieldJSONValue(jsonValFieldsLeft, rel.attrs_left);
    this->buildFieldJSONValue(jsonValFieldsRight, rel.attrs_right);
    jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
    jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;

    return this->removeRelation(jsonVal);
}

/** Removes a relation from redis that is defined as a json object */
bool IndexHandler::removeRelation(Json::Value& jsonVal) {

    this->redisHandler->connect();
    std::string key = this->generateRelationKey(jsonVal[JSON_ATTR_REL_ENTL].asCString(), jsonVal[JSON_ATTR_REL_ENTR].asCString(), md5(jsonVal.toStyledString()));
    Json::Value jsonValReal;
    this->fetchRaw(key, jsonValReal);   // Fetch the actual entry being removed

    if (this->redisHandler->exists(key)) {
        this->redisHandler->decrementKey(KEY_TOTAL_RELATIONS, jsonValReal[JSON_ATTR_REL_COUNT].asInt());    // decrement the global relation count
        this->redisHandler->deleteKey(key);
        return true;
    }
    return false;
}


/**
 * Writes relation to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeRelation(Relation& rel) {
    Json::Value jsonVal;
    std::string key;
    Json::Value jsonValFieldsLeft;
    Json::Value jsonValFieldsRight;

    jsonVal[JSON_ATTR_REL_ENTL] = rel.name_left;
    jsonVal[JSON_ATTR_REL_ENTR] = rel.name_right;

    this->buildFieldJSONValue(jsonValFieldsLeft, rel.attrs_left);
    this->buildFieldJSONValue(jsonValFieldsRight, rel.attrs_right);

    jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
    jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;
    jsonVal[JSON_ATTR_REL_CAUSE] = rel.cause;

    this->redisHandler->connect();
    key = this->generateRelationKey(rel.name_left, rel.name_right, md5(jsonVal.toStyledString()));

    if (this->redisHandler->exists(key)) {
        if (this->fetchRaw(key, jsonVal)) {
            jsonVal[JSON_ATTR_REL_COUNT] = jsonVal[JSON_ATTR_REL_COUNT].asInt() + 1;
        } else
            return false;
    } else
        jsonVal[JSON_ATTR_REL_COUNT] = 1;

    this->redisHandler->incrementKey(KEY_TOTAL_RELATIONS, 1);
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
    this->redisHandler->connect();
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
    return json[JSON_ATTR_ENT_FIELDS].isMember(field);
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

    // Validate that strings are indeed alpha-numeric otherwise return on order supplied
    boost::regex e("^[a-zA-Z0-9]*$");
    if (!boost::regex_match(s1.c_str(), e) || !boost::regex_match(s1.c_str(), e)) return s1 + KEY_DELIMETER + s2;

    std::string ret = "";
    sortedItems.insert(s1);
    sortedItems.insert(s2);
    it = sortedItems.begin(); ret = *it + KEY_DELIMETER; it++; ret += *it;
    return ret;
}

/**
 *  Filter matching relations based on contents attrs.  All of a relations attributes must match those
 *  in the bucket to be included
 */
std::vector<Relation> IndexHandler::filterRelations(
                            std::vector<Relation>& relations, AttributeBucket& filterAttrs) {

    if (filterAttrs.getAttributeHash().size() == 0) return relations;

    bool matching = true;
    std::string key, value;
    std::vector<Relation> filtered_relations;
    AttributeTuple *currAttr, *bucketAttr;

    // Iterate over relations
    for (std::vector<Relation>::iterator it = relations.begin(); it != relations.end(); ++it) {

        // Match left hand relations
        for (valpair::iterator it_inner = it->attrs_left.begin(); it_inner != it->attrs_left.end(); ++it_inner) {
            currAttr = new AttributeTuple(it->name_left, std::get<0>(*it_inner), std::get<1>(*it_inner));
            if (bucketAttr = filterAttrs.getAttribute(*currAttr)) {
                matching = AttributeTuple::compare(*bucketAttr, *currAttr);
                if (!matching) break;
            }
        }

        // Match right hand relations - only process if left hand relations matched
        if (matching)
            for (valpair::iterator it_inner = it->attrs_right.begin(); it_inner != it->attrs_right.end(); ++it_inner) {
                currAttr = new AttributeTuple(it->name_right, std::get<0>(*it_inner), std::get<1>(*it_inner));
                if (bucketAttr = filterAttrs.getAttribute(*currAttr)) {
                    matching = AttributeTuple::compare(*bucketAttr, *currAttr);
                    if (!matching) break;
                }
            }

        if (matching)
            filtered_relations.push_back(*it);
    }
    return filtered_relations;
}

/**
 *  Override for json relation vectors
 *
 *  Filter matching relations based on contents attrs.  All of a relations attributes must match those
 *  in the bucket to be included
 */
std::vector<Json::Value> IndexHandler::filterRelations(
                            std::vector<Json::Value>& relations, AttributeBucket& filterAttrs) {

    if (filterAttrs.getAttributeHash().size() == 0) return relations;

    bool matching = true;
    std::string key, value;
    std::vector<Json::Value> filtered_relations;
    AttributeTuple *currAttr;
    std::unordered_map<std::string, AttributeTuple> hm = filterAttrs.getAttributeHash();

    // Not very efficient (O(n*m)), but OK if filterAttrs is small (m << n)
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it)
        for (std::unordered_map<std::string, AttributeTuple>::iterator it_inner = hm.begin(); it_inner != hm.end(); ++it) {

            // Match left hand relations
            if ((*it)[JSON_ATTR_REL_FIELDSL].isMember(it_inner->second.attribute) &&
                    std::strcmp((*it)[JSON_ATTR_REL_ENTL].asCString(), it_inner->second.entity.c_str()) == 0) {
                currAttr = new AttributeTuple(it_inner->second.entity, it_inner->second.attribute,
                                                (*it)[JSON_ATTR_REL_ENTL][it_inner->second.attribute].asCString());
                matching = AttributeTuple::compare(it_inner->second, *currAttr);
                if (!matching) break;
            }

            // Match right hand relations - only process if left hand relations matched
            if (matching)
                if ((*it)[JSON_ATTR_REL_FIELDSL].isMember(it_inner->second.attribute) &&
                        std::strcmp((*it)[JSON_ATTR_REL_ENTR].asCString(), it_inner->second.entity.c_str()) == 0) {
                    currAttr = new AttributeTuple(it_inner->second.entity, it_inner->second.attribute,
                                                    (*it)[JSON_ATTR_REL_ENTR][it_inner->second.attribute].asCString());
                    matching = AttributeTuple::compare(it_inner->second, *currAttr);
                    if (!matching) break;
                }

            if (matching)
                filtered_relations.push_back(*it);
       }

    return filtered_relations;
}

/** Fetch the number of relations existing */
long IndexHandler::getRelationCountTotal() {
    this->redisHandler->connect();
    return atol(this->redisHandler->read(KEY_TOTAL_RELATIONS).c_str());
}

/** Fetch the number of relations existing */
void IndexHandler::setRelationCountTotal(long value) {
    this->redisHandler->connect();
    this->redisHandler->write(KEY_TOTAL_RELATIONS, std::to_string(value).c_str());
}

/** Takes a list of relations represented as a json vector and returns a relation vector */
std::vector<Relation> IndexHandler::Json2RelationVector(std::vector<Json::Value> fields) {
    std::vector<Relation> relations;
    Relation* relation;
    for (std::vector<Json::Value>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        relation = new Relation();
        relation->fromJSON(*it);
        relations.push_back(*relation);
    }
    return relations;
}

/** Takes a list of relations represented as a relation vector and returns a json vector */
std::vector<Json::Value> IndexHandler::Relation2JsonVector(std::vector<Relation> fields) {
    std::vector<Json::Value> jsonRelations;
    for (std::vector<Relation>::iterator it = fields.begin() ; it != fields.end(); ++it)
        jsonRelations.push_back(it->toJson());
    return jsonRelations;
}

long IndexHandler::computeRelationsCount(std::string left_entity, std::string right_entity) {
    std::vector<Json::Value> relations = this->fetchRelationPrefix(left_entity, right_entity);
    long totalCount = 0;
    for (std::vector<Json::Value>::iterator it = relations.begin() ; it != relations.end(); ++it)
        totalCount += (*it)[JSON_ATTR_REL_COUNT].asInt();
    return totalCount;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
