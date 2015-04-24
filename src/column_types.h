/*
 *  column_types.h
 *
 *  Defines the column UDT.  In memory model for value types.
 *
 *  Created by Ryan Faulkner on 2014-06-16
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _column_types_h
#define _column_types_h

#include <string>
#include <boost/regex.hpp>
#include "emit.h"

#define COLTYPE_NAME_BASE "base"
#define COLTYPE_NAME_NULL "null"
#define COLTYPE_NAME_INT "integer"
#define COLTYPE_NAME_FLOAT "float"
#define COLTYPE_NAME_STR "string"

using namespace std;


/**
 *  Base class for column types
 */
class ColumnBase {
    void* value;
public:
    ColumnBase() { this->value = NULL; }
    void* getValue() { return &(this->value); }
    void setValue(void* value) { this->value = value; }

    /** Returns the object type */
    virtual std::string getType() { return COLTYPE_NAME_BASE; }

    /** Validates the string value as being of this type */
    virtual bool validate(std::string value) { return true; }

    // TODO - overload operators
};


/**
 *  Integer column type
 */
class NullColumn : public ColumnBase {
    void* value;
public:
    NullColumn() { this->value = NULL; }
    string getType() { return COLTYPE_NAME_NULL; }
    bool validate(std::string value) { return true; }
};


/**
 *  Integer column type
 */
class IntegerColumn : public ColumnBase {
    int value;
public:
    IntegerColumn() { this->value = 0; }
    IntegerColumn(int value) { this->value = value; }
    string getType() { return COLTYPE_NAME_INT; }

    bool validate(std::string value) {
        boost::regex e("^[0-9]+$");
        return boost::regex_match(value.c_str(), e);;
    }
};


/**
 *  Float column type
 */
class FloatColumn : public ColumnBase {
    float value;
public:
    FloatColumn() { this->value = 0.0; }
    FloatColumn(float value) { this->value = value; }
    string getType() { return COLTYPE_NAME_FLOAT; }

    bool validate(std::string value) {
        boost::regex e("^[-+]?[0-9]*\\.?[0-9]+$");
        return boost::regex_match(value.c_str(), e);
    }

    // TODO - overload operators
};


/**
 *  String column type
 */
class StringColumn : public ColumnBase {
    string value;
public:
    StringColumn() { this->value = ""; }
    StringColumn(string value) { this->value = value; }
    string getType() { return COLTYPE_NAME_STR; }

    /** Match non whitespace (for now ...) */
    bool validate(std::string value) {
        boost::regex e("[^ ]+$");
        return boost::regex_match(value.c_str(), e);;
    }

    // TODO - overload operators
};

/** Determines whether a string value is a valid type indicator */
bool isValidType(std::string typeStr) {
    if (typeStr.compare(COLTYPE_NAME_INT) == 0 || typeStr.compare(COLTYPE_NAME_FLOAT) == 0 || typeStr.compare(COLTYPE_NAME_STR) == 0)
        return true;
    else
        return false;
}

/** Perform validation of a type value given the type indicator and the value */
bool validateType(std::string typeStr, std::string value) {
    if (typeStr.compare(COLTYPE_NAME_INT) == 0) {
        return IntegerColumn().validate(value);
    } else if (typeStr.compare(COLTYPE_NAME_FLOAT) == 0) {
        return FloatColumn().validate(value);
    } else if (typeStr.compare(COLTYPE_NAME_STR) == 0) {
        return StringColumn().validate(value);
    } else {
        return false;
    }
}

#endif
