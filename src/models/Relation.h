/*
 *  model.h
 *
 *  Defines models for databayes artifacts, primarily entities and relations.
 *
 *  Created by Ryan Faulkner on 2015-01-13
 *
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _relation_h
#define _relation_h

#include "model_def.h"
#include "Entity.h"

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

    Relation() {
        this->types_left = std::unordered_map<std::string, std::string>();
        this->types_right = std::unordered_map<std::string, std::string>();
        this->name_left = "";
        this->name_right = "";
        this->attrs_left = valpair();
        this->attrs_right = valpair();
        this->cause = "";
        this->instance_count = 1;
    }

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

        this->types_left = types_left;
        this->types_right = types_right;
        this->name_left = e1.name;
        this->name_right = e2.name;
        this->attrs_left = attrs_left;
        this->attrs_right = attrs_right;
        this->cause = e1.name;
        this->instance_count = 1;
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

    /** Build relation from Json */
    Relation fromJSON(Json::Value value) {

        // Ensure that all of the relation fields are present
        if (!value.isMember(JSON_ATTR_REL_ENTL) ||
            !value.isMember(JSON_ATTR_REL_ENTR) ||
            !value.isMember(JSON_ATTR_REL_FIELDSL) ||
            !value.isMember(JSON_ATTR_REL_FIELDSR))
            // TODO - perhaps flag an error or add the relevant fields here
            return *this;

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
        for (Json::Value::Members::iterator it = members.begin();
             it != members.end(); ++it)
            if (std::strcmp(JSON_ATTR_FIELDS_COUNT, it->c_str()) != 0)
                // Determine whether a type is being added or an element
                if (it->find(JSON_ATTR_REL_TYPE_PREFIX) == 0) {
                    types_right.insert(std::make_pair(
                        it->substr(1, it->length()),
                        value[JSON_ATTR_REL_FIELDSR][*it].asCString()));
                } else {
                    attrs_right.push_back(
                        std::make_pair(*it, value[JSON_ATTR_REL_FIELDSR]
                            [*it].asCString()));
                }
        return *this;
    }

    /** Handles forming the json for field vectors in the index */
    void buildFieldJSONValue(Json::Value& value, valpair& fields,
                             std::unordered_map<std::string,
                             std::string>& types) {
        int count = 0;
        for (valpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
            value[it->first] = it->second;
            if (types.find(it->first) != types.end())
                value[std::string(JSON_ATTR_REL_TYPE_PREFIX) + it->first] =
                    types[it->first];
            else
                value[std::string(JSON_ATTR_REL_TYPE_PREFIX) + (*it).first] =
                    COLTYPE_NAME_NULL;

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

        this->buildFieldJSONValue(jsonValFieldsLeft, this->attrs_left,
            this->types_left);
        this->buildFieldJSONValue(jsonValFieldsRight, this->attrs_right,
            this->types_right);

        jsonVal[JSON_ATTR_REL_FIELDSL] = jsonValFieldsLeft;
        jsonVal[JSON_ATTR_REL_FIELDSR] = jsonValFieldsRight;
        jsonVal[JSON_ATTR_REL_CAUSE] = this->cause;
        jsonVal[JSON_ATTR_REL_COUNT] = this->instance_count;

        return jsonVal;
    }

    /**
     * Attempts to fetch an entry from index
     */
    bool composeJSON(RedisHandler& rds, Json::Value& json) {
        Json::Reader reader;
        bool parsedSuccess;
        parsedSuccess = reader.parse(rds.read(this->generateKey()), json, false);
        if (parsedSuccess)
            return true;
        else
            return false;
    }

    /** Fetch an attribute value from the relation
            TODO - return something more informative than "" on fail
    */
    std::string getValue(std::string entity, std::string attribute) {
        if (this->name_left.compare(entity) == 0) {
            for (valpair::iterator it = attrs_left.begin();
                    it != attrs_left.end(); ++it)
                if (it->first.compare(attribute) == 0)
                    return it->second;
        } else if (this->name_right.compare(entity) == 0) {
            for (valpair::iterator it = attrs_right.begin();
                    it != attrs_right.end(); ++it)
                if (it->first.compare(attribute) == 0 )
                    return it->second;
        }
        // Attribute not found
        return std::string("");
    }

    /* ORM methods */

    /* Adds a left-hand attribute */
    void addLeftAttribute(std::string name, std::string value, std::string type) {
        this->attrs_left.push_back(std::make_pair(name, value));
        this->types_left.insert(std::make_pair(name, type));
    }

    /* Adds a right-hand attribute */
    void addRightAttribute(std::string name, std::string value, std::string type) {
        this->attrs_right.push_back(std::make_pair(name, value));
        this->types_right.insert(std::make_pair(name, type));
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

    /** Create unique relation hash */
    std::string generateHash() {
        Json::Value val = this->toJson();
        std::string out;
        std::vector<std::string> keys;

        out += std::string(val[JSON_ATTR_REL_ENTL].asCString());
        out += std::string(val[JSON_ATTR_REL_ENTR].asCString());
        out += std::string(val[JSON_ATTR_REL_CAUSE].asCString());

        // Concat left field types and values
        keys = val[JSON_ATTR_REL_FIELDSL].getMemberNames();
        for (std::vector<std::string>::iterator it = keys.begin();
            it != keys.end(); ++it) {
            if (it->compare(JSON_ATTR_FIELDS_COUNT) != 0) {
                out += *it;
                out += std::string(val[JSON_ATTR_REL_FIELDSL][*it].asCString());
            }
        }

        // Concat right field types and values
        keys = val[JSON_ATTR_REL_FIELDSR].getMemberNames();
        for (std::vector<std::string>::iterator it = keys.begin();
            it != keys.end(); ++it) {
            if (it->compare(JSON_ATTR_FIELDS_COUNT) != 0) {
                out += *it;
                out += std::string(val[JSON_ATTR_REL_FIELDSR][*it].asCString());
            }
        }

        return md5(out);
    }

    /** Orders parameters alphanumerically then combines into one string */
    std::string orderPairAlphaNumeric(std::string s1, std::string s2) {
        std::set<std::string> sortedItems;
        std::set<std::string>::iterator it;

        // Validate that strings are indeed alpha-numeric otherwise return on
        // order supplied
        boost::regex e("^[a-zA-Z0-9]*$");
        if (!boost::regex_match(s1.c_str(), e) ||
            !boost::regex_match(s1.c_str(), e))
            return s1 + KEY_DELIMETER + s2;

        std::string ret = "";
        sortedItems.insert(s1);
        sortedItems.insert(s2);
        it = sortedItems.begin(); ret = *it + KEY_DELIMETER; it++; ret += *it;
        return ret;
    }

    /** Generate a key for an entity entry in the index */
    std::string generateKey() {
        std::string rel("rel");
        std::string delim(KEY_DELIMETER);
        return rel + delim + this->orderPairAlphaNumeric(
            this->name_left, this->name_right) + delim + this->generateHash();
    }

    int getInstanceCount(RedisHandler& rds) {
        Json::Value value;
        if (this->composeJSON(rds, value))
            return value[JSON_ATTR_REL_COUNT].asInt();
        else
            return 0;
    }

    bool existsInIndex(RedisHandler& rds) {
        std::string key = this->generateKey();
        return rds.exists(key);
    }

    void write(RedisHandler& rds) {
        std::string key = this->generateKey();
        int instance_count = 0;
        if (rds.exists(key))
            instance_count = this->getInstanceCount(rds);
        Json::Value jsonVal = this->toJson();
        jsonVal[JSON_ATTR_REL_COUNT] = instance_count + 1;
        rds.incrementKey(KEY_TOTAL_RELATIONS, 1);
        rds.write(key, jsonVal.toStyledString());
    }

    bool decrementCount(RedisHandler& rds, int decVal) {
        std::string key = this->generateKey();
        if (rds.exists(key)) {
            Json::Value jsonVal = this->toJson();
            // TODO - issue a warning if the decValue exceeds
            if (decVal > jsonVal[JSON_ATTR_REL_COUNT].asInt()) {
                jsonVal[JSON_ATTR_REL_COUNT] = 0;
                rds.decrementKey(KEY_TOTAL_RELATIONS,
                    jsonVal[JSON_ATTR_REL_COUNT].asInt());
            } else {
                jsonVal[JSON_ATTR_REL_COUNT] = instance_count - decVal;
                rds.decrementKey(KEY_TOTAL_RELATIONS, decVal);
            }
        }
        return false;
    }

    bool remove(RedisHandler& rds) {
        std::string key = this->generateKey();
        if (rds.exists(key)) {
            rds.decrementKey(KEY_TOTAL_RELATIONS, this->instance_count);
            rds.deleteKey(key);
            return true;
        }
        return false;
    }
};

#endif
