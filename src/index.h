/*
 *  index.h
 *
 *  Defines interface for the index.  Provides an in memory instance to fetch results quickly and
 *  provides an interface to disk storage where needed.  Use a heap to maintain order.
 *
 *  Created by Ryan Faulkner on 2014-09-20
 *
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _index_h
#define _index_h

#include <iostream>
#include <fstream>
#include <string>

#define IDX_SIZE 100000

// Index types (entities, relations)
#define IDX_TYPE_ENT 0
#define IDX_TYPE_REL 1


using namespace std;

// TODO - implement heap UDT to handle

class IndexHandler {

    string* inMemEnt;  // In memory instance for entities
    string* inMemRel;  // In memory instance for relations

    int currEnt;
    int currRel;
    int* size;

    int diskBlock;

public:
    /**
     * Constructor and Destructor for index handler
     */
    IndexHandler() {
        this->inMemEnt = new string [IDX_SIZE];
        this->inMemRel = new string [IDX_SIZE];
        this->currEnt = 0;
        this->currRel = 0;
        this->size = new int[2];
        this->size[0] = 0;
        this->size[1] = 0;
    }
    ~IndexHandler() { delete [] inMemEnt; delete [] inMemRel; }

    bool write(int const , string);
    bool writeToDisk(int);

    bool fetch(int const, string);
    string* fetchAll(int const);
    bool fetchFromDisk(int);   // Loads disk

    /**
     *  Getter for index size
     */
    int idxSize(int type) {
        if (type != IDX_TYPE_ENT || type != IDX_TYPE_REL)
            return 0;
        return this->idxSize[type];
    }
};


/**
 * Handles writes to in memory index
 */
bool IndexHandler::write(int const type, string entry) {
    // TODO - handle full in-memory index
    // TODO - Maintain sort order, heap

    // Determine the index type
    if (type == IDX_TYPE_ENT)
        this->inMemEnt[this->currEnt++] = entry;
    else if (type == IDX_TYPE_REL)
        this->inMemRel[this->currRel++] = entry;
    else // bad type
        return false;
    return true;
}

/**
 * Handles writes to disk with strategy
 *
 *  TODO - currently null
 */
bool IndexHandler::writeToDisk(int type) { return false; }

/**
 * Attempts to fetch an entry from index
 *
 *  TODO - this is currently incredibly inefficient, improve
 */
bool IndexHandler::fetch(int const type, string entry) {

    string* inMem;
    int curr;

    // Determine the index type
    if (type == IDX_TYPE_ENT) {
        inMem = this->inMemEnt;
        curr = this->currEnt;
    } else if (type == IDX_TYPE_REL) {
        inMem = this->inMemRel;
        curr = this->currRel;
    } else // bad type
        return false;

    // Find the entry
    for (int i = 0; i < curr; i++)
        if (entry == this->inMemEnt[i])
            return true;
    return false;
}

/**
 * Fetch all entities
 *
 *  TODO - this only returns the in-memory index
 */
string* IndexHandler::fetchAll(int const type) {

    // Determine the index type
    if (type == IDX_TYPE_ENT) {
        return this->inMemEnt;
    } else if (type == IDX_TYPE_REL) {
        return this->inMemRel;
    } else // bad type
        return NULL;
}

/**
 * Handles writes to in memory index
 *
 *  TODO - currently null
 */
bool IndexHandler::fetchFromDisk(int type) { return false; }

#endif
