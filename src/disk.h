/*
 *  disk.cpp
 *
 *  Handles disk interface for storage
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _disk_h
#define _disk_h

#include <iostream>
#include <fstream>
#include <string>
#include "record.h"

using namespace std;

class DiskHandler {
    
public:
    DiskHandler();
    bool writeRecord(Record);
    string getRecord();
};

DiskHandler::DiskHandler() {}


DiskHandler::writeRecord(Record rec) {
    
}

#endif
