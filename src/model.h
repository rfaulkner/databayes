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

#include "md5.h"

#include <string>
#include <unordered_map>
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
    Relation() {}

    Relation(std::string left, std::string right, valpair& attrs_left, valpair& attrs_right) {
        this->name_left = left;
        this->name_right = right;
        this->attrs_left = attrs_left;
        this->attrs_right = attrs_right;
    }

    /** Build relation from Json */
    bool fromJSON(Json::Value value) {

        // Ensure that all of the realtion fields are present
        if (!value.isMember(JSON_ATTR_REL_ENTL) || !value.isMember(JSON_ATTR_REL_ENTR) || !value.isMember(JSON_ATTR_REL_FIELDSL) || !value.isMember(JSON_ATTR_REL_FIELDSR))
            return false;

        this->name_left = value[JSON_ATTR_REL_ENTL].asCString();
        this->name_right = value[JSON_ATTR_REL_ENTR].asCString();

        Json::Value left = value[JSON_ATTR_REL_FIELDSL];
        Json::Value right = value[JSON_ATTR_REL_FIELDSL];

        // Extract the left-hand & right-hand fields
        Json::Value::Members members = value[JSON_ATTR_REL_FIELDSL].getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            if (std::strcmp(JSON_ATTR_FIELDS_COUNT, it->c_str()) != 0)
                attrs_left.push_back(
                    std::make_pair(*it, value[JSON_ATTR_REL_FIELDSL][*it].asCString()));

        members = value[JSON_ATTR_REL_FIELDSR].getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            if (std::strcmp(JSON_ATTR_FIELDS_COUNT, it->c_str()) != 0)
                attrs_left.push_back(
                    std::make_pair(*it, value[JSON_ATTR_REL_FIELDSR][*it].asCString()));
        return true;
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

        return jsonVal;
    }
};

/**
 *  Structure for storing the identifiable properties of an Attribute and matching.
 *
 *  TODO - operator overloading
 */
class AttributeTuple {

public:

    AttributeTuple(std::string entity, std::string attribute, std::string value) {
        this->entity = entity;
        this->attribute = attribute;
        this->value = value;
    }

    ColumnBase type;
    std::string entity;
    std::string attribute;
    std::string value;
    std::string comparator;

    // Performs comparison on an AttributeTuple argument
    bool doCompare(AttributeTuple& attrIn) { return true; }
};

/**
 *  Wrapper around a json object that stores a series of attributes
 */
class AttributeBucket {

    // Hashmap storing attribute instances in this bucket
    std::unordered_map<std::string, AttributeTuple&> attrs;

public:

    // Insert a new atribute into the bucket
    void addAttribute(AttributeTuple& attr) { attrs.insert(std::make_pair<std::string, AttributeTuple&>(this->makeKey(attr), attr)); }

    // Remove an existing attribute from the bucket
    bool removeAttribute(AttributeTuple&) { return attrs.erase(this->makeKey(attr)) == 0; }

    // Does the attribute exist in this bucket?
    AttributeTuple* getAttribute(AttributeTuple& attr) {
        if (this->isAttribute(attr))
            return this->attrs[this->makeKey(attr)];
        else
            return NULL;
    }

    // Does the attribute exist in this bucket?
    bool isAttribute(AttributeTuple& attr) {
        if (this->attrs.find(this->makeKey(attr)) != this->attrs.end())
            return true;
        else
            return false;
    }

    std::string makeKey(AttributeTuple& attr) { return md5(attr.entity + attr.attribute + attr.value); }
};

#endif