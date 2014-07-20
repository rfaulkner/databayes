/*
 *  entity.h
 *
 *  Models entities
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _entity_h
#define _entity_h

#include "column_types.h"

using namespace std;

class Entity {

    std::string name;
    ColumnBase* fields;

public:
    Parser();
    Parser(std:string name);

    void setName(std:string name);

    void setField(std::string name, ColumnBase type);
    void setField(int index, std::string name, ColumnBase type);

    void setFieldValue(std::string name, ColumnBase type);
    void setFieldValue(int index, ColumnBase value);

    ColumnBase& getFieldValue(std::string name);
    ColumnBase& getFieldValue(int index);

    std::string getFieldName(int index);

    std::string getStringVersion();
    // Override << operator?
};
#endif