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
#include <sstream>
#include <unordered_map>

#include "column_types.h"

#define STR_CMD_ADD "ADD"
#define STR_CMD_GET "GET"
#define STR_CMD_GEN "GEN"
#define STR_CMD_CON "CONTSTRAIN"
#define STR_CMD_REL "REL"
#define STR_CMD_DEF "DEF"

#define BAD_INPUT -1

#define SYM_TABLE_ENTITY "ENTITY"
#define SYM_TABLE_FIRLD "FIELD"

using namespace std;


/**
 *  Write a function to handle splitting strings on a delimeter
 */
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

/**
 *  Split + factory method for string splitting
 */
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GET REL E1[(x_1=v_1, x_2=v_2, ...)] [E2(y_1=u_1, y_2=u_2, ...)]
 *      (3) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (3) DEF E1[(x_1=v_1, x_2=v_2, ...)]
 *
 *  (1) provides a facility for insertion into the system, (2) for fetching existing relations, and (3) yields
 *  a generative function given an entity.
 *
 */
class Parser {
    int state;
    std::string currEntity;
    unordered_map<std::string, string>* entityTable;
    unordered_map<std::string, ColumnBase>* fieldTable;

public:
    Parser();
    bool parse(const string&);
    bool analyze(const string&);
    bool checkSymbolTable(const string&, const std::string&);
    void addSymbolTable(const std::pair<std::string, string> elem, const std::string&);
    vector<string> tokenize(const string &source, const char *delimiter = " ", bool keepEmpty = false);

    bool processElem(const string &source);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->state = 0;
    this->symbol_table = new unordered_map<string, string>();
}


/**
 *  Parse loop, calls analyzer
 */
bool Parser::parse(const string& s) {
    
    vector<string> tokens = this->tokenize(s);
    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        if (!this->analyze(*it))
            return false;
    }
    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
bool Parser::analyze(const std::string& s) {

    if (this->state == 0) {
        if (s.compare(STR_CMD_ADD) == 0)
            this->state = 1;
        else if (s.compare(STR_CMD_GET) == 0)
            this->state = 2;
        else if (s.compare(STR_CMD_GEN) == 0)
            this->state = 3;
        else if (s.compare(STR_CMD_DEF) == 0)
            this->state = 5;

    } else if (this->state == 1 || this->state == 2 || this->state == 3) {
        if (s.compare(STR_CMD_REL) == 0)
            this->state = 4;
        else
            return BAD_INPUT;

    } else if (this->state == 4) {

        //  1. Check if s contains a left bracket .. split off the pre-string
        std::string entity;
        std::string field;

        if (s.find("(")) {  // '(' found ready to begin reading fields
            this->state == 6;
            std::vector<string> elems = split(s, '(');
            entity = *elems.begin();

        } else {
            return BAD_INPUT;
        }

        //  2. Check entity symbol table, Ensure the entity exists
        if (!this->checkSymbolTable(*elems.begin(), SYM_TABLE_ENTITY))
            return BAD_INPUT;

        // Process any fields
        this->processField(elems[1]);

    } else if (this->state == 5) {  // DEFINING new entities

        //  1. Check if s contains a left bracket .. split off the pre-string
        std::vector<string> elems = split(s, '(');
        this->currEntity = *elems.begin();

        this->state == 7
        // this->addSymbolTable(std::pair<std::string, string>());

    } else if (this->state == 6) {  // Continue processing fields
        this->processField(s);
    }

    return true;
}


/**
 *  Check for the existence of non-terminal symbols
 */
bool Parser::checkSymbolTable(const string& s, const std::string& tableName) {
    if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        return this->entityTable->end() == this->symbol_table->find(s);
    } else if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        return this->fieldTable->end() == this->symbol_table->find(s);
    }
}


/**
 *  Add a new non-terminal symbol
 */
void Parser::addSymbolTable(const std::pair<std::string, string> elem, const std::string& tableName) {
    if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        this->entityTable->insert(elem);
    } else if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        this->fieldTable->insert(elem);
    }
}


/**
 *  Tokenizes a string for parsing
 */
vector<string> Parser::tokenize(const string &source, const char *delimiter, bool keepEmpty)
{
    vector<string> results;
    
    size_t prev = 0;
    size_t next = 0;
    
    while ((next = source.find_first_of(delimiter, prev)) != string::npos) {
        if (keepEmpty || (next - prev != 0)) {
            results.push_back(source.substr(prev, next - prev));
        }

        // Detect last chunk
        if (prev == next++)
            break;
        else
            prev = next++;
    }
    
    if (prev < source.size()) {
        results.push_back(source.substr(prev));
    }
    
    return results;
}


/**
 *  Handle entity fields
 */
bool Parser::processField(const string &field) {

    std::vector<string> fields = split(s, ',');
    std::string field;
    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;
        if (field.find(")")) {
            this->state = 10;   // Done processing
            if (field.length() == 1)
                return true;
            return this->checkSymbolTable(*elems.begin(), SYM_TABLE_ENTITY)
        } else {
            return this->checkSymbolTable(*elems.begin(), SYM_TABLE_ENTITY)
        }
    }
    return true;
}

#endif