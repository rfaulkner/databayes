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

#define STATE_START 0       // Start state
#define STATE_ADD 10        // Add a new relation
#define STATE_ADD_P1_BEGIN 11
#define STATE_ADD_P1 12
#define STATE_ADD_P2_BEGIN 13
#define STATE_ADD_P2 14
#define STATE_GET 20        // Get relation between two entities with optional conditions
#define STATE_GET_REL 21
#define STATE_GEN 30        // Generate an entity given others
#define STATE_GEN_REL 31
#define STATE_DEF 40        // Describes entity definitions
#define STATE_DEF_P1 41        // Describes entity definitions
#define STATE_FINISH 99     // Successfukl end state

using namespace std;


/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GET REL E1[(x_1=v_1, x_2=v_2, ...)] [E2(y_1=u_1, y_2=u_2, ...)]
 *      (3) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (4) DEF E1[(x_1=v_1, x_2=v_2, ...)]
 *
 *  (1) provides a facility for insertion into the system, (2) for fetching existing relations, (3) yields
 *  a generative function given an entity, and (4) allows the creation of new entities.
 *
 */
class Parser {
    int state;
    bool fieldsProcessed;
    std::string currEntity;
    std::string errStr;
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
    this->errStr = "";
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
        if (!this->analyze(*it)) {
            cout << this->errStr << endl;
            return false;
        }
    }
    this->state = STATE_START;
    this->errStr = "";
    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
bool Parser::analyze(const std::string& s) {

    if (this->state == STATE_START) {
        if (s.compare(STR_CMD_ADD) == 0)
            this->state = STATE_ADD;
        else if (s.compare(STR_CMD_GET) == 0)
            this->state = STATE_GET;
        else if (s.compare(STR_CMD_GEN) == 0)
            this->state = STATE_GEN;
        else if (s.compare(STR_CMD_DEF) == 0)
            this->state = STATE_DEF;

    } else if (this->state == STATE_ADD || this->state == STATE_GET || this->state == STATE_GEN) {
        if (s.compare(STR_CMD_REL) == 0)
            switch (this->state) {
            case STATE_ADD:
                this->state = STATE_ADD_P1_BEGIN;
                break;
            case STATE_GET:
                this->state = STATE_GET_REL;
                break;
            case STATE_GEN:
                this->state = STATE_GEN_REL;
                break;
            }
        else
            return BAD_INPUT;

    } else if (this->state == STATE_ADD_P1_BEGIN || this->state == STATE_ADD_P2_BEGIN) {
        this->parseEntitySymbol(s);

    } else if (this->state == STATE_ADD_P1 || this->state == STATE_ADD_P2) {  // Continue processing fields
        this->parseEntityFields(s);

    } else if (this->state == STATE_GET_REL) {

    } else if (this->state == STATE_GEN_REL) {

    } else if (this->state == STATE_DEF) {  // DEFINING new entities
        this->parseEntitySymbol(s);
        this->state == STATE_DEF_ARGS;

    } else if (this->state == STATE_DEF_ARGS) {
        this->parseEntityFields(s);

        std::string hash = "";
        if (!this->addSymbolTable(std::pair<std::string, string>(hash, entity), SYM_TABLE_ENTITY))
            return BAD_INPUT;

    } else if (this->state == STATE_FINISH) {  // Ensure processing is complete - no symbols should be left at this point
        return BAD_EOL;
    }

    if (this->state == STATE_FINISH)
        switch (this->state) {
            case STATE_DEF:
        }
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
    bool symbolsValid = true;

    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;

        if (this->fieldsProcessed == true) {    // Processing should be complete
            return false;
        }

        if (field.compare(")") == 0) {
            this->fieldsProcessed = true;   // Done processing
            return symbolsValid;

        } else if (field.find(')') == field.length() - 1) {
            this->fieldsProcessed = true;   // Done processing

            field = field.substr(0, field.length() - 1);
            this->parserCmd.append(" ");
            this->parserCmd.append(field);

            symbolsValid = symbolsValid && this->checkSymbolTable(field, SYM_TABLE_FIELD);
            return symbolsValid;

        } else if (field.find(')') < field.length() - 1) { // no chars after '('
            this->errStr = "No symbols permitted after ')'";
            return false;

        } else {
            this->parserCmd.append(" ");
            this->parserCmd.append(field);
            symbolsValid = symbolsValid && this->checkSymbolTable(field, SYM_TABLE_FIELD);
        }
    }
    return symbolsValid;
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


/**
 *  Parses the entity value
 */
void Parser::parseEntitySymbol(std::string s) {

        //   Check if s contains a left bracket .. split off the pre-string
        std::string entity;
        std::string field;
        std::vector<string> elems;

        if (s.find("(")) {  // '(' found ready to begin reading fields

            if (this->state == STATE_ADD_P1_BEGIN)
                this->state == STATE_ADD_P1;
            else if (this->state == STATE_ADD_P2_BEGIN)
                this->state == STATE_ADD_P2;

            elems = this->tokenize(s, '(');
            entity = *elems.begin();
            this->fieldsProcessed = false;

        } else {
            return BAD_INPUT;
        }

        //  Check entity symbol table, Ensure the entity exists
        if (this->state != STATE_DEF) {
            if (!this->checkSymbolTable(entity, SYM_TABLE_ENTITY))
                return BAD_INPUT;
        }

        // Add the entity to the parse command
        this->parserCmd.append(entity);

        // Process any fields
        this->processField(elems[1]);
}

/**
 *  Parses the fields of an entity expression
 */
void Parser::parseEntityFields(std::string s) {
    this->processField(s);

    // Check if fields have been processed for each arg and transition accordingly
    if (this->fieldsProcessed == true && this->state == STATE_ADD_P1)
        this->state = STATE_ADD_P2_BEGIN;
    else if (this->fieldsProcessed == true && this->state == STATE_ADD_P2)
        this->state = STATE_FINISH;
}

#endif