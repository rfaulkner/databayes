
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

#define BAD_CMD "Bad command."

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
        cout << "databayes> ";
        getline (cin, line);
        parser->parse(line);
    }
    return 0;
}
