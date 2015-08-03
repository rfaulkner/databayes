/*
 *  model.h
 *
 *  Defines models for databayes artifacts, primarily entities and relations.
 *
 *  Created by Ryan Faulkner on 2015-01-13
 *
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _model_def_h
#define _model_def_h

#include "../md5.h"
#include "../emit.h"
#include "../column_types.h"
#include "../redis.h"

#include <string>
#include <unordered_map>
#include <json/json.h>
#include <boost/regex.hpp>

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

#define ATTR_TUPLE_COMPARE_EQ "="
#define ATTR_TUPLE_COMPARE_LT "<"
#define ATTR_TUPLE_COMPARE_GT ">"
#define ATTR_TUPLE_COMPARE_LTE "<="
#define ATTR_TUPLE_COMPARE_GTE ">="
#define ATTR_TUPLE_COMPARE_NE "!="

#define KEY_DELIMETER "+"
#define KEY_TOTAL_RELATIONS "total_relations"


using namespace std;

// Vector type that defines entities
typedef std::vector<std::pair<ColumnBase, std::string>> defpair;

// Vector type that defines a set of assignment pairs
typedef std::vector<std::pair<std::string, std::string>> valpair;

#endif
