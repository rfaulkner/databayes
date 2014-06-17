/*
 *  parse.cpp
 *
 *  Handles parsing rules for QL
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _parse_h
#define _parse_h

#include <iostream>
#include <string>

using namespace std;

class Parser {
public:
    Parser();
    bool parse(string);
};

Parser::Parser() {}

#endif