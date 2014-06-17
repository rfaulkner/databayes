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
    ColumnBase() {
        this->value = NULL;
    }
    void* getValue() {
        return &(this->value);
    }
    void setValue(void* value) {
        this->value = value;
    }
    string getType() {
        return COLTYPE_NAME_BASE;
    }
};


/**
 *  Integer column type
 */
class IntegerColumn : public ColumnBase {
    int value;
public:
    IntegerColumn(int value) {
        this->value = value;
    }
    string getType() {
        return COLTYPE_NAME_INT;
    }

};


/**
 *  Float column type
 */
class FloatColumn : public ColumnBase {
    float value;
public:
    FloatColumn(float value) {
        this->value = value;
    }
    string getType() {
        return COLTYPE_NAME_FLOAT;
    }
};


/**
 *  String column type
 */
class StringColumn : public ColumnBase {
    string value;
public:
    StringColumn(float value) {
        this->value = value;
    }
    string getType() {
        return COLTYPE_NAME_STR;
    }

};


#endif
