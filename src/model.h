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
#include "emit.h"

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
#define JSON_ATTR_REL_TYPE_PREFIX "#"

using namespace std;

// Vector type that defines entities
typedef std::vector<std::pair<ColumnBase, std::string>> defpair;

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
 *  Models relations which consist of two entity and a variable number of attribute assignments
 */
class Relation {
public:

    std::unordered_map<std::string, std::string> types_left;
    std::unordered_map<std::string, std::string> types_right;

    std::string name_left;
    std::string name_right;

    valpair attrs_left;
    valpair attrs_right;

    std::string cause;
    int instance_count;

    /** Constructors/Builders for relations  */

    // From Json
    Relation(Json::Value val) { this->fromJSON(val); }

    // Relation
    Relation(
        Entity& e1,
        Entity& e2,
        valpair& attrs_left,
        valpair& attrs_right,
        std::unordered_map<std::string, std::string> types_left,
        std::unordered_map<std::string, std::string> types_right) {

        Relation(e1.name, e2.name, attrs_left, attrs_right, types_left, types_right);
    }

    Relation(
        std::string left,
        std::string right,
        valpair& attrs_left,
        valpair& attrs_right,
        std::unordered_map<std::string, std::string> types_left,
        std::unordered_map<std::string, std::string> types_right) {

        this->types_left = types_left;
        this->types_right = types_right;
        this->name_left = left;
        this->name_right = right;
        this->attrs_left = attrs_left;
        this->attrs_right = attrs_right;
        this->cause = left;
        this->instance_count = 1;
    }

    /** Render the relation state as a string */
    std::string stringify() {
        std::string s;
        s +=  this->name_left + std::string(" / ") + this->name_right + std::string("; ");
        s += std::string(" L ");
        for (valpair::iterator it = attrs_left.begin() ; it != attrs_left.end(); ++it)
            s += it->first + std::string(":") + types_left[it->first] + std::string(":") + it->second;
        s += std::string("; R ");
        for (valpair::iterator it = attrs_right.begin() ; it != attrs_right.end(); ++it)
            s += it->first + std::string(":") + types_right[it->first] + std::string(":") + it->second;
        return s;
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
    bool setInstanceCount(int count) {
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
        Json::Value right = value[JSON_ATTR_REL_FIELDSR];

        // Extract the left-hand & right-hand fields
        Json::Value::Members members = value[JSON_ATTR_REL_FIELDSL].getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            // Determine whether a type is being added or an element
            if (std::strcmp(JSON_ATTR_FIELDS_COUNT, it->c_str()) != 0) {
                if (it->find(JSON_ATTR_REL_TYPE_PREFIX) == 0) {
                    types_left.insert(std::make_pair(it->substr(1, it->length()), value[JSON_ATTR_REL_FIELDSL][*it].asCString()));
                } else {
                    attrs_left.push_back(std::make_pair(*it, value[JSON_ATTR_REL_FIELDSL][*it].asCString()));
                }
            }
        members = value[JSON_ATTR_REL_FIELDSR].getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            if (std::strcmp(JSON_ATTR_FIELDS_COUNT, it->c_str()) != 0)
                // Determine whether a type is being added or an element
                if (it->find(JSON_ATTR_REL_TYPE_PREFIX) == 0) {
                    types_right.insert(std::make_pair(it->substr(1, it->length()), value[JSON_ATTR_REL_FIELDSR][*it].asCString()));
                } else {
                    attrs_right.push_back(
                        std::make_pair(*it, value[JSON_ATTR_REL_FIELDSR][*it].asCString()));
                }
        return true;
    }

    /** Handles forming the json for field vectors in the index */
    void buildFieldJSONValue(Json::Value& value, valpair& fields, std::unordered_map<std::string, std::string>& types) {
        int count = 0;
        for (valpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
            value[it->first] = it->second;
            if (types.find(it->first) != types.end())
                value[std::string(JSON_ATTR_REL_TYPE_PREFIX) + it->first] = types[it->first];
            else
                value[std::string(JSON_ATTR_REL_TYPE_PREFIX) + (*it).first] = COLTYPE_NAME_NULL;

            count++;
        }
        value[JSON_ATTR_FIELDS_COUNT] = count;
    }

    /** Convert relation object to JSON */
    Json::Value toJson() {

        Json::Value jsonVal;
        Json::Value jsonValFieldsLeft;
        Json::Value jsonValFieldsRight;

        jsonVal[JSON_ATTR_REL_ENTL] = this->name_left;
        jsonVal[JSON_ATTR_REL_ENTR] = this->name_right;

        this->buildFieldJSONValue(jsonValFieldsLeft, this->attrs_left, this->types_left);
        this->buildFieldJSONValue(jsonValFieldsRight, this->attrs_right, this->types_right);

        jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
        jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;
        jsonVal[JSON_ATTR_REL_CAUSE] = this->cause;
        jsonVal[JSON_ATTR_REL_COUNT] = this->instance_count;

        return jsonVal;
    }
};

/**
 *  Structure for storing the identifiable properties of an Attribute and matching.
 */
class AttributeTuple {

public:

    std::string type;
    std::string entity;
    std::string attribute;
    std::string value;
    std::string comparator;

    AttributeTuple() {}

    AttributeTuple(std::string entity, std::string attribute, std::string value, std::string type) {
        this->entity = entity;
        this->attribute = attribute;
        this->value = value;
        this->type = type;
    }

    /** Format object as string */
    std::string toString() {
        Json::Value json;
        json["entity"] = this->entity;
        json["attribute"] = this->attribute;
        json["value"] = this->value;
        json["comparator"] = this->comparator;
        return json.toStyledString();
    }

    /** From string compose object attributes */
    void fromString(std::string formattedJson) {
        Json::Reader reader;
        Json::Value json;
        bool parsedSuccess;
        parsedSuccess = reader.parse(formattedJson, json, false);

        if (parsedSuccess) {
            this->entity = json["entity"].asCString();
            this->attribute = json["attribute"].asCString();
            this->value = json["value"].asCString();
            this->comparator = json["comparator"].asCString();
         } else
            emitCLIError("Could not parse json from formatted string.");
    }

    /** Switching logic for attribute tuples - defaults to true in absence of a defined comparator */
    static bool compare(AttributeTuple& lhs, AttributeTuple& rhs) {
        if (std::strcmp(lhs.comparator.c_str(), "<") == 0)
            return lhs < rhs;
        else if (std::strcmp(lhs.comparator.c_str(), ">") == 0)
            return lhs > rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "<=") == 0)
            return lhs <= rhs;
        else if (std::strcmp(lhs.comparator.c_str(), ">=") == 0)
            return lhs >= rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "!=") == 0)
            return lhs != rhs;
        else if (std::strcmp(lhs.comparator.c_str(), "=") == 0)
            return lhs == rhs;
        else    // Error - unrecognized operator
            return true;
    }


    bool operator>(const AttributeTuple &rhs) const {
        // convert value to column type and make comparison
        if (strcmp(rhs.type.c_str(), COLTYPE_NAME_INT) == 0) {
            return atoi(this->value.c_str()) > atoi(rhs.value.c_str());
        } else if (strcmp(rhs.type.c_str(), COLTYPE_NAME_FLOAT) == 0) {
            return atof(this->value.c_str()) > atof(rhs.value.c_str());
        } else if (strcmp(rhs.type.c_str(), COLTYPE_NAME_STR) == 0) {
            return this->value.c_str()[0] > rhs.value.c_str()[0];
        }

        // For unknown types emit a warning and return false
        emitCLIWarning("Could not recognize type \"" + rhs.type + std::string("\"for: ") + rhs.entity + std::string(".") + rhs.attribute);
        return false;
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
        if (strcmp(other.type.c_str(), COLTYPE_NAME_INT) == 0) {
            return atoi(this->value.c_str()) == atoi(other.value.c_str());
        } else if (strcmp(other.type.c_str(), COLTYPE_NAME_FLOAT) == 0) {
            return atof(this->value.c_str()) == atof(other.value.c_str());
        } else if (strcmp(other.type.c_str(), COLTYPE_NAME_STR)) {
            return strcmp(this->value.c_str(), other.value.c_str()) == 0;
        }

        // For unknown types emit a warning and return false
        emitCLIWarning(std::string("Could not recognize type \"") + other.type + std::string("\"for: ") + other.entity + std::string(".") + other.attribute);
        return false;
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
    std::unordered_map<std::string, std::string> attrs;

public:

    AttributeBucket() {}

    /** Constructor that converts valpair list to AttributeBucket */
    AttributeBucket(std::string entity, valpair& vals, std::unordered_map<std::string, std::string>& types) { this->addAttributes(entity, vals, types); }

    /** Insert a new atribute into the bucket */
    void addAttribute(AttributeTuple attr) { this->attrs.insert(std::make_pair(this->makeKey(attr), attr.toString())); }

    /** Insert a list of attributes */
    void addAttributes(std::string entity, valpair& vals, std::unordered_map<std::string, std::string>& types) {
        AttributeTuple tuple;
        for (valpair::iterator it = vals.begin() ; it != vals.end(); ++it) {
            tuple = AttributeTuple(entity, std::get<0>(*it), std::get<1>(*it), types[std::get<0>(*it)]);
            this->addAttribute(tuple);
        }
    }

    // Remove an existing attribute from the bucket
    bool removeAttribute(AttributeTuple& attr) { return this->attrs.erase(this->makeKey(attr)) == 0; }

    // Get hashmap copy
    std::unordered_map<std::string, std::string> getAttributeHash() { return attrs; }

    // Does the attribute exist in this bucket?
    AttributeTuple getAttribute(AttributeTuple& attr) {
        if (this->isAttribute(attr)) {
            attr.fromString(this->attrs[this->makeKey(attr)]);
            return attr;
        } else {
            emitCLIError("Couldn't find attribute in bucket.");
            return AttributeTuple();
        }
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