

/*
 *  client.cpp
 *
 *  Handles the client env.
 *
 *  Created by Ryan Faulkner on 2014-06-08
 *  Copyright (c) 2014. All rights reserved.
 */

#include <iostream>
#include <string>
#include <redis3m/connection.h>
#include "parse.h"

using namespace std;

bool handleUserInput(string input) {
    return true;
}

int main() {
    string line;
    Parser* parser = new Parser();
    parser->setDebug(true);

    // Read the input
    while (1) {
        // 1. Poll redis
        // 2. Fetch next command off the queue
        parser->parse(line);
        parser->resetState();
        // 3. execute timeout
    }
    return 0;
}

