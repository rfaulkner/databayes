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

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() { this->redisHandler = new RedisHandler(REDISHOST, REDISPORT); }
    ~IndexHandler() { delete redisHandler; }

    bool writeEntity(Entity&);
    bool writeRelation(Relation&);
    bool writeRelation(Json::Value&);
    bool writeToDisk(int);

    bool removeEntity(std::string);
    bool removeEntity(Entity&);
    bool removeRelation(Relation&);
    bool removeRelation(Json::Value&);

    bool composeJSON(std::string, Json::Value&);

    bool existsEntity(std::string);
    bool existsEntity(Entity&);
    bool existsEntityField(std::string, std::string);
    bool existsRelation(std::string, std::string);
    bool existsRelation(Relation&);

    bool fetchRaw(std::string, Json::Value&);
    bool fetchEntity(std::string, Json::Value&);
    std::string fetchEntityFieldType(std::string, std::string);
    std::vector<Json::Value> fetchRelationPrefix(std::string, std::string);
    std::vector<Json::Value> fetchPatternJson(std::string);
    std::vector<std::string> fetchPatternKeys(std::string);
    bool fetchFromDisk(int);   // Loads disk

    std::vector<Relation> Json2RelationVector(std::vector<Json::Value>);
    std::vector<Json::Value> Relation2JsonVector(std::vector<Relation>);

    void filterRelations(std::vector<Relation>&, AttributeBucket&);
    void filterRelations(std::vector<Json::Value>&, AttributeBucket&);
    std::vector<Json::Value> fetchAttribute(AttributeTuple&);

    std::string generateEntityKey(std::string);
    std::string generateRelationKey(std::string, std::string, std::string);
    std::string generateRelationHash(Json::Value);

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

/** Generates a hash from the JSON representation of a relation */
std::string IndexHandler::generateRelationHash(Json::Value val) {
    std::string out;
    std::vector<std::string> keys;

    out += std::string(val[JSON_ATTR_REL_ENTL].asCString());
    out += std::string(val[JSON_ATTR_REL_ENTR].asCString());
    out += std::string(val[JSON_ATTR_REL_CAUSE].asCString());

    // Concat left field types and values
    keys = val[JSON_ATTR_REL_FIELDSL].getMemberNames();
    for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); ++it) {
        if (it->compare(JSON_ATTR_FIELDS_COUNT) != 0) {
            out += *it;
            out += std::string(val[JSON_ATTR_REL_FIELDSL][*it].asCString());
        }
    }

    // Concat right field types and values
    keys = val[JSON_ATTR_REL_FIELDSR].getMemberNames();
    for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); ++it) {
        if (it->compare(JSON_ATTR_FIELDS_COUNT) != 0) {
            out += *it;
            out += std::string(val[JSON_ATTR_REL_FIELDSR][*it].asCString());
        }
    }

    return md5(out);
}

/** Handles forming the json for field vectors in the index */
void IndexHandler::buildFieldJSONDefinition(Json::Value& value, defpair& fields) {
    int count = 0;
    for (defpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
        value[(*it).second] = (*it).first.getType();
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
bool IndexHandler::removeEntity(Entity& e) { this->removeEntity(e.name); }

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
    Json::Value val = rel.toJson();
    return this->removeRelation(val);
}

/** Removes a relation from redis that is defined as a json object */
bool IndexHandler::removeRelation(Json::Value& jsonVal) {

    this->redisHandler->connect();
    std::string key = this->generateRelationKey(jsonVal[JSON_ATTR_REL_ENTL].asCString(), jsonVal[JSON_ATTR_REL_ENTR].asCString(), generateRelationHash(jsonVal));
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
    Json::Value val = rel.toJson();
    return this->writeRelation(val);
}

/**
 * Writes relation to in memory index.
 *
 *  e.g. {"entity": <string:entname>, "fields": <string_array:[<f1,f2,...>]>}
 */
bool IndexHandler::writeRelation(Json::Value& jsonVal) {
    std::string key;
    this->redisHandler->connect();
    key = this->generateRelationKey(std::string(jsonVal[JSON_ATTR_REL_ENTL].asCString()), std::string(jsonVal[JSON_ATTR_REL_ENTR].asCString()), generateRelationHash(jsonVal));

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
    Json::Value json;
    for (std::vector<string>::iterator it = keys.begin(); it != keys.end(); ++it) {
        json = Json::Value();
        this->composeJSON(this->redisHandler->read(*it), json);
        relations.push_back(json);
    }
    return relations;
}

/** Fetch a set of relations matching the entities */
std::vector<Json::Value> IndexHandler::fetchAttribute(AttributeTuple& attr) {
    this->redisHandler->connect();
    std::vector<std::string> keysl = this->redisHandler->keys(this->generateRelationKey(attr.entity, "*", "*"));
    std::vector<std::string> keysr = this->redisHandler->keys(this->generateRelationKey("*", attr.entity, "*"));
    std::vector<Json::Value> relations;

    for (std::vector<string>::iterator it = keysl.begin(); it != keysl.end(); ++it) {
        Json::Value json;
        this->composeJSON(this->redisHandler->read(*it), json);
        relations.push_back(json);
    }
    for (std::vector<string>::iterator it = keysr.begin(); it != keysr.end(); ++it) {
        Json::Value json;
        this->composeJSON(this->redisHandler->read(*it), json);
        relations.push_back(json);
    }
    return relations;
}

bool IndexHandler::existsEntity(Entity& e) { this->existsEntity(e.name); }

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
bool IndexHandler::existsRelation(Relation& r) { this->existsRelation(r.name_left, r.name_right); }

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
std::vector<Json::Value> IndexHandler::fetchPatternJson(std::string pattern) {
    std::vector<Json::Value> elems = std::vector<Json::Value>();
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
            elems.push_back(inMem);
    }
    return elems;
}

/**
 * Fetch all redis keys matching a pattern
 *
 * Returns a vector of matching keys
 */
std::vector<string> IndexHandler::fetchPatternKeys(std::string pattern) {
    std::vector<std::string> elems = std::vector<std::string>();
    std::vector<string> vec;
    this->redisHandler->connect();
    vec = this->redisHandler->keys(pattern);
    for (std::vector<std::string>::iterator it = vec.begin() ; it != vec.end(); ++it)
        elems.push_back((*it).substr(4, (*it).length()));
    return elems;
}

/** Ensure that the field type is valid */
bool IndexHandler::validateEntityFieldType(std::string entity, std::string field, std::string value) {
    bool valid = true;
    Json::Value json;
    valid = valid && this->fetchEntity(entity, json);
    if (valid) valid = valid && (json[JSON_ATTR_ENT_FIELDS].isMember(field.c_str())); // ensure field exists
    if (valid) valid = valid && validateType(json[JSON_ATTR_ENT_FIELDS][field].asCString(), value); // ensure the value is a valid instance of the type
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
 *  in the bucket to be included.  For eax
 */
void IndexHandler::filterRelations(std::vector<Relation>& relations, AttributeBucket& filterAttrs) {

    if (filterAttrs.getAttributeHash().size() == 0) return;

    bool matching = true;
    std::string key, value;
    AttributeTuple currAttr, bucketAttr;
    std::vector<int> killIndices;

    cout << filterAttrs.stringify() << endl;

    // Iterate over relations
    for (std::vector<Relation>::iterator it = relations.begin(); it != relations.end(); ++it) {

        // Match left hand relations
        for (valpair::iterator it_inner = it->attrs_left.begin(); it_inner != it->attrs_left.end(); ++it_inner) {
            currAttr = AttributeTuple(it->name_left, std::get<0>(*it_inner), std::get<1>(*it_inner), it->types_left[std::get<0>(*it_inner)]);
            cout << currAttr.toString() << endl;
            // If the entity is empty (the entity:attr was not on the bucket) continue, otherwise compare
            if (filterAttrs.isAttribute(currAttr)) {
                matching = AttributeTuple::compare(bucketAttr, currAttr);
                if (!matching) break;
            }
        }

        // Match right hand relations - only process if left hand relations matched
        if (matching)
            for (valpair::iterator it_inner = it->attrs_right.begin(); it_inner != it->attrs_right.end(); ++it_inner) {
                currAttr = AttributeTuple(it->name_right, std::get<0>(*it_inner), std::get<1>(*it_inner), it->types_right[std::get<0>(*it_inner)]);
                cout << currAttr.toString() << endl;
                if (filterAttrs.isAttribute(currAttr)) {
                    matching = AttributeTuple::compare(bucketAttr, currAttr);
                    if (!matching) break;
                }
            }

        cout << matching << endl;
        if (!matching)
            killIndices.push_back(std::distance(relations.begin(), it));

    }

    // Wipe the unmatched elements
    for (std::vector<int>::iterator it = killIndices.begin(); it != killIndices.end(); ++it)
        relations.erase(relations.begin() + *it);
}

/**
 *  Override for json relation vectors
 *
 *  Filter matching relations based on contents attrs.  All of a relations attributes must match those
 *  in the bucket to be included
 */
void IndexHandler::filterRelations(std::vector<Json::Value>& relations, AttributeBucket& filterAttrs) {

    if (filterAttrs.getAttributeHash().size() == 0) return;

    bool matching = true;
    std::string key, value;
    AttributeTuple currAttr, filterAttr;
    std::unordered_map<std::string, std::vector<std::string>> hm = filterAttrs.getAttributeHash();

    // Not very efficient (O(n*m)), but OK if filterAttrs is small (m << n)
    for (std::vector<Json::Value>::iterator it = relations.begin(); it != relations.end(); ++it) {
        for (std::unordered_map<std::string, std::vector<std::string>>::iterator it_inner = hm.begin(); it_inner != hm.end(); ++it_inner) {
            for (std::vector<std::string>::iterator it_tuples = it_inner->second.begin(); it_tuples != it_inner->second.end(); ++it_tuples) {

                // Get filter attribute from hashmap values from bucket
                filterAttr = AttributeTuple();
                filterAttr.fromString(*it_tuples);

                // Match left hand relations
                if ((*it)[JSON_ATTR_REL_FIELDSL].isMember(filterAttr.attribute) &&
                        std::strcmp((*it)[JSON_ATTR_REL_ENTL].asCString(), filterAttr.entity.c_str()) == 0) {

                    currAttr = AttributeTuple(filterAttr.entity, filterAttr.attribute,
                                (*it)[JSON_ATTR_REL_FIELDSL][filterAttr.attribute].asCString(),
                                std::string(JSON_ATTR_REL_TYPE_PREFIX) + std::string((*it)[JSON_ATTR_REL_FIELDSL][filterAttr.attribute].asCString())
                                );
                    matching = AttributeTuple::compare(filterAttr, currAttr);
                    if (!matching) break;
                }

                // Match right hand relations - only process if left hand relations matched
                if (matching)
                    if ((*it)[JSON_ATTR_REL_FIELDSR].isMember(filterAttr.attribute) &&
                            std::strcmp((*it)[JSON_ATTR_REL_ENTR].asCString(), filterAttr.entity.c_str()) == 0) {
                        currAttr = AttributeTuple(filterAttr.entity, filterAttr.attribute,
                                    (*it)[JSON_ATTR_REL_FIELDSR][filterAttr.attribute].asCString(),
                                    std::string(JSON_ATTR_REL_TYPE_PREFIX) + std::string((*it)[JSON_ATTR_REL_FIELDSR][filterAttr.attribute].asCString())
                                    );
                        matching = AttributeTuple::compare(filterAttr, currAttr);
                        if (!matching) break;
                    }
            }

            if (!matching) relations.erase(relations.begin() + std::distance(relations.begin(), it));
        }
    }
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
    for (std::vector<Json::Value>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        Relation r = Relation(*it);
        relations.push_back(r);
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

/** Fetches the Column Type of a particular entity field */
std::string IndexHandler::fetchEntityFieldType(std::string entity, std::string field) {
    Json::Value val;
    this->fetchEntity(entity, val);
    std::vector<std::string> keys = val[JSON_ATTR_ENT_FIELDS].getMemberNames();
    for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); ++it)
        if (val[JSON_ATTR_ENT_FIELDS][*it].isString() && std::strcmp(it->c_str(), field.c_str()) == 0)
            if (isValidType(val[JSON_ATTR_ENT_FIELDS][*it].asCString()))
                return val[JSON_ATTR_ENT_FIELDS][*it].asCString();
            else
                return std::string(COLTYPE_NAME_NULL);
    return std::string(COLTYPE_NAME_NULL);
}

#endif
