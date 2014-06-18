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

#include <string>
#include <vector>
#include "column_types.h"


using namespace std;

class Record {
    vector<ColumnBase> columns;
    int num_cols;
public:
    Record(int);
    Record(vector<ColumnBase>);
    bool setRecord(vector<ColumnBase>);
    vector<ColumnBase> getRecord();
};

Record::Record(int num_cols) {
    this->num_cols = num_cols;
}

Record::Record(vector<ColumnBase> columns) {
    this->columns = columns;
    this->num_cols = columns.size();
}

bool Record::setRecord(vector<ColumnBase> columns) {
    // TODO - exception handling on case columns isn't an array
    if (columns.size() == this->num_cols) {
        this->columns = columns;
        return true;
    } else {
        return false;
    }
}

vector<ColumnBase> Record::getRecord() {
    return this>columns;
}

#endif
