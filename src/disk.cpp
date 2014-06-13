
/*
 *  disk.cpp
 *
 *  Handles disk interface for storage
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#include <iostream>
#include <string>

using namespace std;

class DiskHandler {
    
public:
    DiskHandler();
    bool writeRecord();
    string getRecord();
};

DiskHandler::DiskHandler() {}

