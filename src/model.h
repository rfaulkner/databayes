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
#define JSON_ATTR_REL_CAUSE "cause"

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

/**
 *  Models relations which consist of
 */
class Relation {
public:

    std::string name_left;
    std::string name_right;
    valpair attrs_left;
    valpair attrs_right;

    std::string cause;
    long instance_count;

    /** Constructor/Builder for relations  */
    Relation() {}

    Relation(std::string left, std::string right, valpair& attrs_left, valpair& attrs_right) {
        this->name_left = left;
        this->name_right = right;
        this->attrs_left = attrs_left;
        this->attrs_right = attrs_right;
        this->cause = left;
        this->instance_count = 1;
    }

    /** Set the causal entity */
    bool setCause(std::string cause) {
        if (std::strcmp(cause.c_str(), this->name_left.c_str()) != 0 && std::strcmp(cause.c_str(), this->name_right.c_str()) != 0)
            return false;
        else if (std::strcmp(cause.c_str(), this->name_left.c_str()) == 0) {
            this->cause = this->name_left;
        } else {
            this->cause = this->name_right;
        }
        return true;
    }

    /** Set the instance count */
    bool setInstanceCount(long count) {
        if (count < 1)
            return false;
        else
            this->instance_count = count;
        return true;
    }


    /** Build relation from Json */
    bool fromJSON(Json::Value value) {

        // Ensure that all of the realtion fields are present
        if (!value.isMember(JSON_ATTR_REL_ENTL) || !value.isMember(JSON_ATTR_REL_ENTR) || !value.isMember(JSON_ATTR_REL_FIELDSL) || !value.isMember(JSON_ATTR_REL_FIELDSR))
            return false;

        this->name_left = value[JSON_ATTR_REL_ENTL].asCString();
        this->name_right = value[JSON_ATTR_REL_ENTR].asCString();
        this->cause = value[JSON_ATTR_REL_CAUSE].asCString();
        this->instance_count = value[JSON_ATTR_REL_COUNT].asInt();

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

        jsonVal[JSON_ATTR_REL_CAUSE] = this->cause;
        jsonVal[JSON_ATTR_REL_COUNT] = std::to_string(this->instance_count);

        return jsonVal;
    }
};

/**
 *  Structure for storing the identifiable properties of an Attribute and matching.
 */
class AttributeTuple {

public:

    ColumnBase* type;
    std::string entity;
    std::string attribute;
    std::string value;
    std::string comparator;

    AttributeTuple() {
        this->type = new NullColumn();
    }

    AttributeTuple(std::string entity, std::string attribute, std::string value) {
        this->entity = entity;
        this->attribute = attribute;
        this->value = value;
        this->type = new NullColumn();
    }

    ~AttributeTuple() { delete type; }

    /** Switching logic for attribute tuples */
    static bool compare(AttributeTuple& lhs, AttributeTuple& rhs) {
        if (std::strcmp(lhs.comparator.c_str(), "<"))
            return lhs < rhs;
        else if (std::strcmp(lhs.comparator.c_str(), ">"))
            return lhs > rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "<="))
            return lhs <= rhs;
        else if (std::strcmp(lhs.comparator.c_str(), ">="))
            return lhs >= rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "!="))
            return lhs != rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "="))
            return lhs == rhs;
        else    // Error - unrecogmized operator
            return false;
    }

    bool operator>(const AttributeTuple &rhs) const {
        // convert value to column type and make comparison
        if (strcmp(rhs.type->getType().c_str(), COLTYPE_NAME_INT) == 0) {
            return atoi(this->value.c_str()) > atoi(rhs.value.c_str());
        } else if (strcmp(rhs.type->getType().c_str(), COLTYPE_NAME_FLOAT) == 0) {
            return atof(this->value.c_str()) > atof(rhs.value.c_str());
        } else if (strcmp(rhs.type->getType().c_str(), COLTYPE_NAME_STR) == 0) {
            return this->value.c_str()[0] > rhs.value.c_str()[0];
        }

        // Match by default
        return true;
    }

    bool operator<(const AttributeTuple &rhs) const {
        return rhs > *this;
    }

    bool operator>=(const AttributeTuple &rhs) const {
        return *this > rhs || *this == rhs;
    }

    bool operator<=(const AttributeTuple &rhs) const {
        return *this < rhs || *this == rhs;
    }

    bool operator==(const AttributeTuple &other) const {

        // convert value to column type and make comparison
        if (strcmp(other.type->getType().c_str(), COLTYPE_NAME_INT) == 0) {
            return atoi(this->value.c_str()) == atoi(other.value.c_str());
        } else if (strcmp(other.type->getType().c_str(), COLTYPE_NAME_FLOAT) == 0) {
            return atof(this->value.c_str()) == atof(other.value.c_str());
        } else if (strcmp(other.type->getType().c_str(), COLTYPE_NAME_STR)) {
            return strcmp(this->value.c_str(), other.value.c_str()) == 0;
        }

        // Match by default
        return true;
    }

    bool operator!=(const AttributeTuple &other) const {
        return !(*this==other);
    }
};

/**
 *  Wrapper around a json object that stores a series of attributes
 */
class AttributeBucket {

    // Hashmap storing attribute instances in this bucket
    std::unordered_map<std::string, AttributeTuple> attrs;

public:

    AttributeBucket() {}

    /** Constructor that converts valpair list to AttributeBucket */
    AttributeBucket(std::string entity, valpair& vals) { this->addAttributes(entity, vals); }

    /** Insert a new atribute into the bucket */
    void addAttribute(AttributeTuple& attr) { this->attrs.insert(std::make_pair<std::string, AttributeTuple&>(this->makeKey(attr), attr)); }

    /** Insert a list of attributes */
    void addAttributes(std::string entity, valpair& vals) {
        AttributeTuple* tuple;
        for (valpair::iterator it = vals.begin() ; it != vals.end(); ++it) {
            tuple = new AttributeTuple(entity, std::get<0>(*it), std::get<1>(*it));
            this->addAttribute(*tuple);
        }
    }

    // Remove an existing attribute from the bucket
    bool removeAttribute(AttributeTuple& attr) { return this->attrs.erase(this->makeKey(attr)) == 0; }

    // Get hashmap copy
    std::unordered_map<std::string, AttributeTuple> getAttributeHash() { return attrs; }

    // Does the attribute exist in this bucket?
    AttributeTuple* getAttribute(AttributeTuple& attr) {
        std::string key = this->makeKey(attr);
        if (this->isAttribute(attr))
            return &(this->attrs[key]);
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