/*
 *  Entity.h
 *
 *  Defines models for databayes artifacts, primarily entities and relations.
 *
 *  Created by Ryan Faulkner on 2015-01-13
 *
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _entity_h
#define _entity_h

#include "model_def.h"

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

    // Entity(Json::Value val) { this->fromJSON(val); }

    std::string name;
    defpair attrs;

    /** Create a copy of the Entity in Json */
    Json::Value toJson() {
        Json::Value jsonVal;
        Json::Value fields;
        jsonVal[JSON_ATTR_ENT_ENT] = this->name;
        this->createJSON(fields, this->attrs);
        jsonVal[JSON_ATTR_ENT_FIELDS] = fields;
        return jsonVal;
    }

    /** Render the relation state as a string */
    std::string stringify() {
        std::string s;
        s +=  this->name + std::string("; ");
        for (defpair::iterator it = attrs.begin() ; it != attrs.end(); ++it)
            s += it->first->getType() + std::string(":") + it->second;
        return s;
    }

    void addAttribute(std::string name, ColumnBase colType) {
        attrs.push_back(std::make_pair(&colType, name));
    }

    /** Handles forming the json for field vectors in the index */
    void createJSON(Json::Value& value, defpair& fields) {
        int count = 0;
        for (defpair::iterator it = fields.begin() ; it != fields.end(); ++it) {
            value[it->second] = it->first->getType();
            count++;
        }
        value[JSON_ATTR_FIELDS_COUNT] = count;
    }

    /** Generate a key for an entity entry in the index */
    std::string generateKey() {
        std::string ent("ent");
        std::string delim(KEY_DELIMETER);
        return ent + delim + this->name;
    }

    /** Handles writing the entity JSON representation to redis */
    void write(RedisHandler& rds) {
        Json::Value jsonVal;
        Json::Value jsonValFields;
        jsonVal[JSON_ATTR_ENT_ENT] = this->name;
        this->createJSON(jsonValFields, this->attrs);
        jsonVal[JSON_ATTR_ENT_FIELDS] = jsonValFields;

        rds.connect();
        rds.write(this->generateKey(), jsonVal.toStyledString());
    }

    bool remove(RedisHandler& rds) {
        std::string key = this->generateKey();
        rds.connect();
        if (rds.exists(key)) {
            rds.deleteKey(key);
            return true;
        }
        return false;
    }
};

#endif
