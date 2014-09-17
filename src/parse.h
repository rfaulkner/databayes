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
#define STR_CMD_LST "LST"
#define STR_CMD_ENT "ENT"

#define BAD_INPUT "Bad input symbol"
#define BAD_EOL "Bad end of line"

#define SYM_TABLE_ENTITY "ENTITY"
#define SYM_TABLE_FIELD "FIELD"


// STATE REPRESENTATIONS

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
#define STATE_DEF_PROC_ENTITY 41
#define STATE_DEF_PROC_ATTRS 42

#define STATE_LST 50        // Lists entities or relations
#define STATE_LST_ENT 51        // Lists entities
#define STATE_LST_REL 52        // Lists relations

#define STATE_FINISH 99     // Successful end state

using namespace std;


/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GET REL E1[(x_1=v_1, x_2=v_2, ...)] [E2(y_1=u_1, y_2=u_2, ...)]
 *      (3) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (4) DEF E1[(x_1, x_2, ...)]
 *      (5) LST REL [E1 [E2]]
 *      (6) LST ENT [E1]*
 *
 *  (1) provides a facility for insertion into the system
 *
 *  (2) get a specific relation if it exists
 *
 *  (3) generate a sample conditional on an entity
 *
 *  (4) define a new entity
 *
 *  (5) list relations optionally dependent relational entities
 *
 *  (6) list entities.  Either specify them or simply list all.
 *
 */
class Parser {

    int state;
    int macroState;

    bool fieldsProcessed;
    bool debug;

    bool error;
    std::string errStr;

    std::string currEntity;
    unordered_map<std::string, string>* entityTable;
    unordered_map<std::string, /*ColumnBase*/ string>* fieldTable;
    std::string parserCmd;

public:
    Parser();
    bool parse(const string&);
    void analyze(const string&);
    bool checkSymbolTable(const string&, const std::string&);
    bool addSymbolTable(const std::pair<std::string, string> elem, const std::string&);

    void parseEntitySymbol(std::string);
    void processField(const string &source);

    std::vector<std::string> tokenize(const std::string &source, const char delimiter = ' ');
    std::vector<std::string> &tokenize(const std::string &source, const char delimiter, std::vector<std::string> &elems);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->state = STATE_START;
    this->error = false;
    this->debug = false;
    this->errStr = "";
    this->entityTable = new unordered_map<string, string>();
    this->fieldTable = new unordered_map<string, /* ColumnBase*/ string>();
}


/**
 *  Parse loop, calls analyzer
 */
bool Parser::parse(const string& s) {

    vector<string> tokens = this->tokenize(s);

    this->parserCmd = "";           // Initialize parser command
    this->state = STATE_START;      // Initialize state
    this->error = false;            // Initialize error condition
    this->errStr = "";              // Initialize error message

    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        this->analyze(*it);
        if (this->error) {
            cout << this->errStr << endl;
            return false;
        }
    }
    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
void Parser::analyze(const std::string& s) {

    if (this->state == STATE_START) {
        if (s.compare(STR_CMD_ADD) == 0) {
            this->state = STATE_ADD;
            this->macroState = STATE_ADD;
        } else if (s.compare(STR_CMD_GET) == 0) {
            this->state = STATE_GET;
            this->macroState = STATE_GET;
        } else if (s.compare(STR_CMD_GEN) == 0) {
            this->state = STATE_GEN;
            this->macroState = STATE_GEN;
        } else if (s.compare(STR_CMD_DEF) == 0) {
            this->state = STATE_DEF;
            this->macroState = STATE_DEF;
        } else if (s.compare(STR_CMD_LST) == 0) {
            this->state = STATE_LST;
            this->macroState = STATE_LST;
        }

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
        else {
            this->error = true;
            this->errStr = BAD_INPUT;
            return;
        }

    } else if (this->state == STATE_ADD_P1_BEGIN || this->state == STATE_ADD_P2_BEGIN) {
        this->parseEntitySymbol(s);

    } else if (this->state == STATE_ADD_P1 || this->state == STATE_ADD_P2) {  // Continue processing fields
        this->processField(s);

    } else if (this->state == STATE_GET_REL) {

    } else if (this->state == STATE_GEN_REL) {

    } else if (this->state == STATE_DEF) {  // DEFINING new entities
        this->state == STATE_DEF_PROC_ENTITY;

    } else if (this->state == STATE_DEF_PROC_ENTITY) {

        this->parseEntitySymbol(s);

        // Ensure this entity has not already been defined
        if (this->state != STATE_DEF) {
            if (!this->checkSymbolTable(this->currEntity, SYM_TABLE_ENTITY))
                this->error = true;
                this->errStr = BAD_INPUT;
        }

        // Add this entity to the symbol table
        std::string hash = "";
        if (!this->addSymbolTable(std::pair<std::string, string>(hash, this->currEntity), SYM_TABLE_ENTITY)) {
            this->error = true;
            this->errStr = BAD_INPUT;
            return;
        }

        // Process the attributes next.
        this->state = STATE_DEF_PROC_ATTRS;

    } else if (this->state == STATE_DEF_PROC_ATTRS) {
        this->processField(s);
        if (this->fieldsProcessed)
            this->state = STATE_FINISH;

    } else if (this->state == STATE_LST) {
        if (s.compare(STR_CMD_REL) == 0)
            this->state = STATE_LST_REL;
        else if (s.compare(STR_CMD_ENT) == 0)
            this->state = STATE_LST_ENT;

    } else if (this->state == STATE_LST_ENT) {

        // WRITE ENTITIES

        // if the symbol is empty move to the complete state
        // TODO - actually condition on this
        this->state = STATE_FINISH;

    } else if (this->state == STATE_LST_REL) {

        // WRITE RELATIONS

        // if the symbol is empty move to the complete state
        // TODO - actually condition on this
        this->state = STATE_FINISH;

    } else if (this->state == STATE_FINISH) {  // Ensure processing is complete - no symbols should be left at this point
        this->error = true;
        this->errStr = BAD_EOL;
        return;
    }
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
bool Parser::addSymbolTable(const std::pair<std::string, string> elem, const std::string& tableName) {
    if (tableName.compare(SYM_TABLE_ENTITY) == 0) {
        this->entityTable->insert(elem);
    } else if (tableName.compare(SYM_TABLE_FIELD) == 0) {
        this->fieldTable->insert(elem);
    } else {
        return false;
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


/**
 *  Parses the entity value.  If the input string contains subsequent fields these are also parsed.
 *
 *  @param s    input string
 */
void Parser::parseEntitySymbol(std::string s) {

    //   Check if s contains a left bracket .. split off the pre-string
    std::string field;
    std::vector<string> elems;
    bool noFields = true;

    // If the input contains
    if (s.find("(")) {
        noFields = false;
        elems = this->tokenize(s, '(');
        this->currEntity = *elems.begin();
    } else
        this->currEntity = s;

    if (this->debug)
        cout << "Reading entity: " << this->currEntity << endl; // DEBUG
    this->fieldsProcessed = false;

    // Add the entity to the parse command
    this->parserCmd.append(this->currEntity);

    // Process any fields
    if (!noFields)
        this->processField(elems[1]);
}


/**
 *  Handle entity fields
 *
 *  @param string& fieldStr     Consists of one or more comma separated fields possibly terminated with ')'
 */
void Parser::processField(const string &fieldStr) {
    std::vector<string> fields = this->tokenize(fieldStr, ',');
    std::string field;

    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;

        // Processing should be complete
        if (this->fieldsProcessed == true) {
            this->error = true;
            this->errStr = "All fields have already been processed.";
            break;
        }

        // Evaluate fields
        if (field.compare(")") == 0) {
            this->fieldsProcessed = true;   // Done processing

        } else if (field.find(')') == field.length() - 1) { // e.g.
            this->fieldsProcessed = true;
            field = field.substr(0, field.length() - 1);
            this->parserCmd.append(" ");
            this->parserCmd.append(field);

            if (this->debug)
                cout << "Reading field: " << field << endl; // DEBUG

            this->error = this->error && this->checkSymbolTable(field, SYM_TABLE_FIELD);

        } else if (field.find(')') < field.length() - 1) { // no chars after '('
            this->error = true;
            this->errStr = "No symbols permitted after ')'";
            break;

        } else {
            this->parserCmd.append(" ");
            this->parserCmd.append(field);

            if (this->debug)
                cout << "Reading field: " << field << endl; // DEBUG

            this->error = this->error && this->checkSymbolTable(field, SYM_TABLE_FIELD);
        }
    }
}


#endif