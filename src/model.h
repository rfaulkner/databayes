/*
 *  model.h
 *
 *  Defines models for databayes artifacts, primarily entities and relations.
 *
 *  Created by Ryan Faulkner on 2015-01-13
 *
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _model_h
#define _model_h

#include <string>
#include <json/json.h>

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

// Vector type that defines entities
typedef std::vector<std::pair<ColumnBase*, std::string>> defpair;

// Vector type that defines a set of assignment pairs
typedef std::vector<std::pair<std::string, std::string>> valpair;


// Define entity and relation builder classes

class Entity {
public:

    /** Constructor/Builder for relations  */
    Entity(std::string name) { this->name = name; }

    /** Constructor/Builder for relations  */
    Entity(std::string name, defpair& attrs) {
        this->name = name;
        this->attrs = attrs;
    }

    std::string name;
    defpair attrs;
};

class Relation {
public:

    std::string name_left;
    std::string name_right;
    valpair attrs_left;
    valpair attrs_right;

    /** Constructor/Builder for relations  */
    Relation(std::string left, std::string right, valpair& attrs_left, valpair& attrs_right) {
        this->name_left = left;
        this->name_right = right;
        this->attrs_left = attrs_left;
        this->attrs_right = attrs_right;
    }

    /** Handles forming the json for field vectors in the index */
    void getJSONValue(Json::Value& value, valpair& fields) {
        int count = 0;
        for (valpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
            value[(*it).first] = (*it).second;
            count++;
        }
        value[JSON_ATTR_FIELDS_COUNT] = count;
    }

    /** Convert relation object to JSON */
    Json::Value toJson() {

        Json::Value jsonVal;
        std::string key;
        Json::Value jsonValFieldsLeft;
        Json::Value jsonValFieldsRight;

        jsonVal[JSON_ATTR_REL_ENTL] = this->name_left;
        jsonVal[JSON_ATTR_REL_ENTR] = this->name_right;

        this->getJSONValue(jsonValFieldsLeft, this->attrs_left);
        this->getJSONValue(jsonValFieldsRight, this->attrs_right);

        jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
        jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;

        this->redisHandler->connect();
        key = this->generateRelationKey(this->name_left, this->name_right, md5(jsonVal.toStyledString()));

        if (this->redisHandler->exists(key)) {
            if (this->fetchRaw(key, jsonVal)) {
                jsonVal[JSON_ATTR_REL_COUNT] = jsonVal[JSON_ATTR_REL_COUNT].asInt() + 1;
            } else
                return false;
        } else {jsonVal[JSON_ATTR_REL_COUNT] = 1;}

        return jsonVal;
    }
};

/**
 *  Wrapper around a json object that stores a series of attributes
 */
class AttributeBucket {

    Json::Value inMem;
    Json::Reader reader;

public:

    void addAttribute(std::string entity, valpair& vp);
    std::string std::string getAttribute(std::string entity, valpair& vp);

    Json::Value& getJson() {
    }

    void removeAttribute(std::string entity, valpair& vp);

};

#endif