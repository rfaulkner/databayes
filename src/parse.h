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

#define BAD_INPUT "Bad input symbol"
#define BAD_EOL "Bad end of line"

#define SYM_TABLE_ENTITY "ENTITY"
#define SYM_TABLE_FIELD "FIELD"

#define STATE_START 0

using namespace std;


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
    unordered_map<std::string, /*ColumnBase*/ string>* fieldTable;
    std::string parserCmd;

public:
    Parser();
    bool parse(const string&);
    bool analyze(const string&);
    bool checkSymbolTable(const string&, const std::string&);
    void addSymbolTable(const std::pair<std::string, string> elem, const std::string&);

    std::vector<std::string> tokenize(const std::string &source, const char delimiter = ' ');
    std::vector<std::string> &tokenize(const std::string &source, const char delimiter, std::vector<std::string> &elems);

    bool processField(const string &source);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->state = STATE_START;
    this->entityTable = new unordered_map<string, string>();
    this->fieldTable = new unordered_map<string, /* ColumnBase*/ string>();
}


/**
 *  Parse loop, calls analyzer
 */
bool Parser::parse(const string& s) {

    // Initialize parser command
    this->parserCmd = "";

    vector<string> tokens = this->tokenize(s);
    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        if (!this->analyze(*it))
            return false;
    }
    this->state = STATE_START;
    // cout << this->parserCmd << endl;
    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
bool Parser::analyze(const std::string& s) {

    if (this->state == STATE_START) {
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
        std::vector<string> elems;

        if (s.find("(")) {  // '(' found ready to begin reading fields
            this->state == 6;
            elems = this->tokenize(s, '(');
            entity = *elems.begin();

        } else {
            return BAD_INPUT;
        }

        //  2. Check entity symbol table, Ensure the entity exists
        if (!this->checkSymbolTable(*elems.begin(), SYM_TABLE_ENTITY))
            return BAD_INPUT;

        //
        this->parserCmd.append(entity);

        // Process any fields
        this->processField(elems[1]);

    } else if (this->state == 5) {  // DEFINING new entities

        //  1. Check if s contains a left bracket .. split off the pre-string
        std::vector<string> elems = this->tokenize(s, '(');
        this->currEntity = *elems.begin();

        this->state == 7;
        this->addSymbolTable(std::pair<std::string, string>(), SYM_TABLE_ENTITY);

        // TODO - process fields

    } else if (this->state == 6) {  // Continue processing fields
        this->processField(s);

    } else if (this->state == 10) {  // Ensure processing is complete
        return BAD_EOL;
    }

    return true;
}


/**
 *  Check for the existence of non-terminal symbols
 */
bool Parser::checkSymbolTable(const string& s, const std::string& tableName) {
    if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        return this->entityTable->end() == this->entityTable->find(s);
    } else if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        return this->fieldTable->end() == this->fieldTable->find(s);
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
 *  Handle entity fields
 */
bool Parser::processField(const string &fieldStr) {
    std::vector<string> fields = this->tokenize(fieldStr, ',');
    std::string field;
    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;

        if (field.compare(")") == 0) {
            this->state = 10;   // Done processing
            return true;

        } else if (field.find(')')) {
            this->state = 10;   // Done processing

            field = field.substr(0, field.length() - 1);
            this->parserCmd.append(" ");
            this->parserCmd.append(field);

            return this->checkSymbolTable(field, SYM_TABLE_FIELD);

        } else {
            this->parserCmd.append(" ");
            this->parserCmd.append(field);
            return this->checkSymbolTable(field, SYM_TABLE_FIELD);
        }
    }
    return true;
}

/**
 *  Write a function to handle splitting strings on a delimeter
 */
std::vector<std::string> &Parser::tokenize(const std::string &s, const char delim, std::vector<std::string> &elems) {
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
std::vector<std::string> Parser::tokenize(const std::string &s, const char delim) {
    std::vector<std::string> elems;
    this->tokenize(s, delim, elems);
    return elems;
}

#endif