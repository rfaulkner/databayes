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
#include <vector>

using namespace std;

/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GET REL E1[(x_1=v_1, x_2=v_2, ...)] [E2(y_1=u_1, y_2=u_2, ...)]
 *      (3) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *
 *  (1) provides a facility for insertion into the system, (2) for fetching existing relations, and (3) yields
 *  a generative function given an entity.
 *
 */
class Parser {
public:
    Parser();
    bool parse(const string&);
    bool tokenize(const string&, const char*, bool);
};

Parser::Parser() {}


/**
 *  Handles top level
 */
Parser::bool parse(const string& s) {
    
    this->tokenize(s);
    
    // TODO - proceed with SLR parsing
}


/**
 *  Tokenizes a string for parsing
 */
vector<string> Parser::tokenize(const string &source, const char *delimiter = " ", bool keepEmpty = false)
{
    vector<string> results;
    
    size_t prev = 0;
    size_t next = 0;
    
    while ((next = source.find_first_of(delimiter, prev)) != string::npos) {
        if (keepEmpty || (next - prev != 0)) {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next++;
    }
    
    if (prev < source.size()) {
        results.push_back(source.substr(prev));
    }
    
    return results;
}

#endif