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
#include <regex>

#define COLTYPE_NAME_BASE "base"
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
    virtual string getType() { return COLTYPE_NAME_BASE; }

    /** Validates the string value as being of this type */
    virtual bool validate(std::string value) { return true; }
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
        std::regex r4("[0-9]*", std::regex_constants::basic);
        return std::regex_match (value.c_str(), r4));
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
    bool validate(std::string value) { return true; }
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
    bool validate(std::string value) { return true; }
};


/**
 *  Control for returning a column type by string
 */
ColumnBase* getColumnType(string columnType) {
    if (columnType.compare(COLTYPE_NAME_INT) == 0) {
        return new IntegerColumn();
    } else if (columnType.compare(COLTYPE_NAME_FLOAT) == 0) {
        return new FloatColumn();
    } else if (columnType.compare(COLTYPE_NAME_STR) == 0) {
        return new StringColumn();
    } else {
        return NULL;
    }
}

#endif
