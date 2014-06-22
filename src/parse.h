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
#include <unordered_map>
#include "column_types.h"

#define STR_CMD_ADD "ADD"
#define STR_CMD_GET "GET"
#define STR_CMD_GEN "GEN"
#define STR_CMD_CON "CONTSTRAIN"
#define STR_CMD_REL "REL"

#define BAD_INPUT -1

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
    int state;
    unordered_map<std::string, ColumnBase> symbol_table;
public:
    Parser();
    bool parse(const string&);
    bool analyze(const string&);
    bool checkSymbolTable(const string&);
    bool addSymbolTable(const std::pair<std::string, ColumnBase> elem);
    bool tokenize(const string&, const char*, bool);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->state = 0;
    this->symbol_table = new vector<string,ColumnBase>();
}


/**
 *  Parse loop, calls analyzer
 */
bool Parser::parse(const string& s) {
    
    vector<string> tokens = this->tokenize(s);
    std::reverse(tokens.begin(), tokens.end());  // reverse tokens
    
    // Iterate through the input symbols
    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        if (!this->analyze(it))
            return false;
    }
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
bool Parser::analyze(const string& s) {
    switch (*it) {
        case STR_CMD_ADD:
            if (this->state == 0) {
                this->state == 1;   // Transition state
            } else
                return BAD_INPUT;
            break;
        case STR_CMD_GET:
            if (this->state == 0) {
                this->state == 2;   // Transition state
            } else
                return BAD_INPUT;
            break;
        case STR_CMD_GEN:
            if (this->state == 0) {
                this->state == 3;   // Transition state
            } else
                return BAD_INPUT;
            break;
        case STR_CMD_REL:
            if (this->state != 1 || this->state == 2 || this->state == 3) {
                return BAD_INPUT;
            }
            break;
        case STR_CMD_CON:
            // handles inputs of the type -> "GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]"
        case ',':
            break;
        case '(':
            break;
        case ')':
            break;
        default:
            // handle symbol interpretation
            if (this->state != 1 || this->state == 2 || this->state == 3) {
                //  1. Check if s contains a left bracket .. split off the pre-string
                //  2.Check symbol table
                //  3. iterate through params
                //  4. Validate syntax
            }
            break;
    }
    return true;
}


/**
 *  Check for the existence of non-terminal symbols
 */
bool Parser::checkSymbol(const string& s) {
    std::unordered_map<const_iterator>::const_iterator = this->symbol_table.find(s);
}


/**
 *  Add a new non-terminal symbol
 */
bool Parser::addSymbol(const std::pair<std::string, ColumnBase> elem) {
    this->symbol_table.insert(elem);
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