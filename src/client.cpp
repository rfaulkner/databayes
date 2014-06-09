
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

using namespace std;

bool handleUserInput(string input) {
    return true;
}

int main() {
    string line;
    
    // Read the input
    while (1) {
        cout << "databayes> ";
        getline (cin, line);
        handleUserInput(line);
    }
    return 0;
}
