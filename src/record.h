/*
 *  record.h
 *
 *  Defines the record UDT.  A record is a set of Columns plus some functionality.
 *
 *  Created by Ryan Faulkner on 2014-06-16
 *  Copyright (c) 2014. All rights reserved.
 */


#ifndef _record_h
#define _record_h

#include <iostream>
#include <string>
#inclde "column_types.h"


using namespace std;

class Record {
    ColumnBase* columns;
    int num_cols;
public:
    Record(int);
    Record(ColumnBase*);
    bool setRecord(ColumnBase*);
    ColumnBase* getRecord();
};

Record::Record(int num_cols) {
    this->num_cols = num_cols;
}

Record::Record(ColumnBase* columns) {
    this->columns = columns;
    this->num_cols = sizeof(columns) / sizeof(ColumnBase);
}

bool Record::setRecord(ColumnBase* columns) {
    // TODO - exception handling on case columns isn't an array
    if (columns.size() == this->num_cols) {
        this->columns = columns;
        return true;
    } else {
        return false;
    }
}

ColumnBase* Record::getRecord() {
    return->columns;
}

#endif
