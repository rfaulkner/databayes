/*
 *  model.h
 *
 *  Defines models for databayes artifacts, primarily entities and relations.
 *
 *  Created by Ryan Faulkner on 2015-01-13
 *
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _attribute_tuple_h
#define _attribute_tuple_h

#include "model_def.h"

/**
 *  Structure for storing the identifiable properties of an Attribute and matching.
 */
class AttributeTuple {

public:

    std::string type;
    std::string entity;
    std::string attribute;
    std::string value;

    AttributeTuple() {}

    AttributeTuple(std::string formattedJson) {
        this->fromString(formattedJson);
    }

    AttributeTuple(std::string entity, std::string attribute, std::string value,
                   std::string type) {
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
        json["type"] = this->type;
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
            this->type = json["type"].asCString();
        } else
            emitCLIError("Could not parse json from formatted string.");
    }

    /** Switching logic for attribute tuples - defaults to true in absence of a
        defined comparator */
    template <class Type>
    static bool compare(AttributeTuple& lhs, AttributeTuple& rhs,
        std::string comparator) {

        // lhs and rhs must have matching types -- float can be compared to int
        if (lhs.type.compare(rhs.type) != 0 &&
            !((lhs.type.compare(COLTYPE_NAME_INT) == 0 &&
                rhs.type.compare(COLTYPE_NAME_FLOAT) == 0) ||
            (rhs.type.compare(COLTYPE_NAME_INT) == 0 &&
                lhs.type.compare(COLTYPE_NAME_FLOAT) == 0))
           )
           return false;

        // lhs and rhs must both be valid
        if (!validateType(lhs.type, lhs.value) ||
            !validateType(rhs.type, rhs.value))
            return false;

        // Convert the types
        Type lval = Type(lhs.value);
        Type rval = Type(rhs.value);

        if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_LT) == 0)
            return lval < rval;
        else if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_GT) == 0)
            return lval > rval;
        else if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_LTE) == 0)
            return lval <= rval;
        else if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_GTE) == 0)
            return lval >= rval;
        else if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_NE) == 0)
            return lval != rval;
        else if (std::strcmp(comparator.c_str(), ATTR_TUPLE_COMPARE_EQ) == 0)
            return lval == rval;
        else {   // Error - unrecognized operator
            emitCLIError("Unrecognized comparator on Attribute Tuple comparison");
            return false;
        }

    }
};

/**
 *  Wrapper around a json object that stores a series of attributes
 */
class AttributeBucket {

    // Hashmap storing attribute instances in this bucket
    std::unordered_map<std::string, std::vector<std::string>> attrs;

public:

    AttributeBucket() {}

    /** Constructor that converts valpair list to AttributeBucket */
    AttributeBucket(std::string entity,
                    valpair& vals,
                    std::unordered_map<std::string,
                    std::string>& types) {
        this->addAttributes(entity, vals, types);
    }

    /** Insert a new atribute into the bucket */
    void addAttribute(AttributeTuple attr) {
        std::unordered_map<std::string, std::vector<std::string>>::iterator vec = this->attrs.find(this->makeKey(attr));
        if (vec != this->attrs.end())
            vec->second.push_back(attr.toString());
        else {
            std::vector<std::string> newVec;
            newVec.push_back(attr.toString());
            this->attrs.insert(std::make_pair(this->makeKey(attr), newVec));
        }
    }

    /** Insert a list of attributes */
    void addAttributes(std::string entity, valpair& vals, std::unordered_map<std::string, std::string>& types) {
        AttributeTuple tuple;
        for (valpair::iterator it = vals.begin() ; it != vals.end(); ++it) {
            tuple = AttributeTuple(entity, std::get<0>(*it), std::get<1>(*it), types[std::get<0>(*it)]);
            this->addAttribute(tuple);
        }
    }

    // Remove an existing attribute from the bucket
    vector<AttributeTuple> getAttributes(std::string entity, std::string attr) {
        vector<std::string>* tupleStrings;
        vector<AttributeTuple> attrs;
        if (hasKey(entity, attr)) {
            tupleStrings = &(this->attrs[this->makeKey(entity, attr)]);
            for (std::vector<std::string>::iterator it = tupleStrings->begin(); it != tupleStrings->end(); ++it)
                attrs.push_back(AttributeTuple(*it));
        }
        return attrs;
    }

    // Remove an existing attribute from the bucket
    bool removeAttribute(AttributeTuple& attr) { return this->attrs.erase(this->makeKey(attr)) == 0; }

    // Get hashmap copy
    std::unordered_map<std::string, std::vector<std::string>> getAttributeHash() { return attrs; }

    // Does the attribute exist in this bucket?
    bool isAttribute(AttributeTuple& attr) {
        std::vector<std::string> vec;
        AttributeTuple at;
        if (this->hasKey(attr)) {
            vec = this->attrs[this->makeKey(attr)];
            for (std::vector<std::string>::iterator it = vec.begin() ; it != vec.end(); ++it) {
                at.fromString(*it);         // Set the tuple
                if (attr.value.compare(at.value.c_str()) == 0)
                    return true;
            }
        }
        return false;
    }

    // Does the attribute exist in this bucket?
    bool hasKey(std::string entity, std::string attr) {
        if (this->attrs.find(this->makeKey(entity, attr)) != this->attrs.end())
            return true;
        else
            return false;
    }

    // Override hasKey to take an AttributeTuple as an arg
    bool hasKey(AttributeTuple& attr) { return this->hasKey(attr.entity, attr.attribute); }

    // Generate a key in the bucket for this item
    std::string makeKey(AttributeTuple& attr) { return md5(attr.entity + attr.attribute); }

    // Generate a key in the bucket for this item
    std::string makeKey(std::string entity, std::string attr) { return md5(entity + attr); }

    // Enumerate the items in the bucket
    std::string stringify() {
        std::string bucketStr;
        for (std::unordered_map<std::string, std::vector<std::string>>::iterator it = this->attrs.begin(); it != this->attrs.end(); ++it)
            for (std::vector<std::string>::iterator it_in = it->second.begin(); it_in != it->second.end(); ++it_in)
                bucketStr += it->first + std::string(" -> ") + *it_in + std::string("\n");
        return bucketStr;
    }

    void clearBucket() { attrs.clear(); }
};

#endif
