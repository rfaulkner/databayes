//
//  column_types.h
//  
//
//  Created by Ryan Faulkner on 6/16/14.
//
//

#ifndef _column_types_h
#define _column_types_h

#include <string>

using namespace std;

/**
 *  Base class for column types
 */
class ColumnBase {
    void* value = null;
public:
    ColumnBase::ColumnBase() {}
    void* getValue() {
        return &(this->value);
    }
    void setValue(void* value) {
        this->value = value*;
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

};


#endif
