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
#include <algorithm>
#include <json/json.h>

#include "column_types.h"
#include "index.h"

#define STR_CMD_ADD "add"
#define STR_CMD_GET "get"
#define STR_CMD_GEN "gen"
#define STR_CMD_CON "constrain"
#define STR_CMD_REL "rel"
#define STR_CMD_DEF "def"
#define STR_CMD_LST "lst"
#define STR_CMD_ENT "ent"

#define BAD_INPUT "Bad input symbol"
#define BAD_EOL "Bad end of line"
#define ERR_ENT_EXISTS "ERR: Entity already exists."
#define ERR_ENT_NOT_EXISTS "ERR: Entity not found."
#define ERR_ALL_FIELDS_PROC "All fields have already been processed."
#define ERR_NO_SYM_AFTER "No symbols permitted after ')'"
#define ERR_INVALID_FIELD_TYPE "Invalid field type"
#define ERR_ENT_NOT_FOUND "Can't find entity."
#define ERR_ENT_BAD_FORMAT "Invalid Entity assign format"
#define ERR_BAD_FIELD_TYPE "Bad field type on instance."

#define WILDCARD_CHAR '*'


// STATE REPRESENTATIONS

#define STATE_START 0       // Start state

#define STATE_ADD 10        // Add a new relation
#define STATE_ADD_P1 12
#define STATE_ADD_P2 14

#define STATE_GET 20        // Get relation between two entities with optional conditions
#define STATE_GET_REL 21

#define STATE_GEN 30        // Generate an entity given others
#define STATE_GEN_REL 31

#define STATE_DEF 40        // Describes entity definitions
#define STATE_DEF_PROC 41

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

    bool entityProcessed;
    bool fieldsProcessed;
    bool debug;

    bool error;
    std::string errStr;

    // Define lists that store state of newly defined fields and values
    std::vector<std::pair<ColumnBase*, std::string>>* currFields;
    std::vector<std::pair<std::string, std::string>>* currValues;
    std::vector<std::pair<std::string, std::string>>* bufferValues;

    // Define internal state that stores entity handles
    std::string currEntity;
    std::string bufferEntity;

    // Index interface pointer
    IndexHandler* indexHandler;

    // Parse methods
    void parseEntitySymbol(std::string);
    void processFieldStatement(const string &source);
    void parseEntityDefinitionField(std::string);
    void parseEntityAssignField(std::string);

public:
    Parser();

    void setDebug(bool);

    bool parse(const string&);
    void analyze(const string&);

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
    this->indexHandler = new IndexHandler();
    this->errStr = "";
    this->currFields = NULL;
    this->currValues = NULL;
}


/**
 * Setter for debug property
 */
void Parser::setDebug(bool debugVal) {
    this->debug = debugVal;
}


/**
 *  Parse loop, calls analyzer
 */
bool Parser::parse(const string& s) {

    vector<string> tokens = this->tokenize(s);

    this->state = STATE_START;      // Initialize state
    this->error = false;            // Initialize error condition
    this->errStr = "";              // Initialize error message

    // Process command tokens
    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        if (this->debug)
            cout << "DEBUG -- Processing input token: " << *it << endl; // DEBUG
        this->analyze(*it);
        if (this->error) {
            cout << this->errStr << endl;
            return false;
        }
    }

    // Emit the interpreted command
    if (this->debug)
        1; // TODO - output translated input

    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
void Parser::analyze(const std::string& s) {

    // Convert to lower case to enforce case insensitivity
    string sLower = s;
    std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);

    if (this->state == STATE_START) {

        if (sLower.compare(STR_CMD_ADD) == 0) {
            this->state = STATE_ADD;
            this->macroState = STATE_ADD;
        } else if (sLower.compare(STR_CMD_GET) == 0) {
            this->state = STATE_GET;
            this->macroState = STATE_GET;
        } else if (sLower.compare(STR_CMD_GEN) == 0) {
            this->state = STATE_GEN;
            this->macroState = STATE_GEN;
        } else if (sLower.compare(STR_CMD_DEF) == 0) {
            this->state = STATE_DEF;
            this->macroState = STATE_DEF;
        } else if (sLower.compare(STR_CMD_LST) == 0) {
            this->state = STATE_LST;
            this->macroState = STATE_LST;
        }

    } else if (this->state == STATE_ADD || this->state == STATE_GET || this->state == STATE_GEN) {
        if (sLower.compare(STR_CMD_REL) == 0)
            switch (this->state) {
            case STATE_ADD:
                this->state = STATE_ADD_P1;
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

    } else if (this->state == STATE_ADD_P1 || this->state == STATE_ADD_P2) {

        // Determine if entity or fields need to be processed
        if (this->entityProcessed) {
            this->parseEntitySymbol(sLower);
        } else {
            if (this->currValues != NULL) delete this->currValues;
            this->currValues = new vector<std::pair<std::string, std::string>>;
            this->parseEntitySymbol(sLower);

            // Ensure that entities exist
            if (!this->indexHandler->existsEntity(this->currEntity)) {
                this->error = true;
                this->errStr = ERR_ENT_NOT_EXISTS;
                this->state = STATE_FINISH;
                return;
            }
        }

        // If all fields have been processed transition
        if (this->fieldsProcessed)
            if (this->state == STATE_ADD_P1) {
                this->state = STATE_ADD_P2;
                this->bufferEntity = this->currEntity;
                this->bufferValues = this->currValues;
                this->currValues = new vector<std::pair<std::string, std::string>>;
                this->fieldsProcessed = false;
                this->entityProcessed = false;

            } else if (this->state == STATE_ADD_P2) {
                this->state = STATE_FINISH;
            }

    } else if (this->state == STATE_GET_REL) {

    } else if (this->state == STATE_GEN_REL) {

    } else if (this->state == STATE_DEF) {  // DEFINING new entities

        this->currFields = new vector<std::pair<ColumnBase*, std::string>>;
        this->state == STATE_DEF_PROC;
        this->parseEntitySymbol(sLower);

        // Ensure this entity has not already been defined
        if (this->indexHandler->existsEntity(this->currEntity)) {
            this->error = true;
            this->errStr = ERR_ENT_EXISTS;
            this->state = STATE_FINISH;
            return;
        }

        if (this->fieldsProcessed)
            this->state = STATE_FINISH;

    } else if (this->state == STATE_DEF_PROC) {
        this->processFieldStatement(sLower);
        if (this->fieldsProcessed)
            this->state = STATE_FINISH;

    } else if (this->state == STATE_LST) {
        if (sLower.compare(STR_CMD_REL) == 0)
            this->state = STATE_LST_REL;
        else if (sLower.compare(STR_CMD_ENT) == 0)
            this->state = STATE_LST_ENT;

    } else if (this->state == STATE_LST_ENT) {

        std::vector<Json::Value>* entities;

        // Write Entities
        this->parseEntitySymbol(sLower);

        cout << "Current Matched Entities for \"" << this->currEntity << "\""<< endl;

        entities = this->indexHandler->fetchPatternJson("ent_" + this->currEntity);
        if (entities != NULL) {
            for (std::vector<Json::Value>::iterator it = entities->begin() ; it != entities->end(); ++it) {
                // cout << "-> " << it->toStyledString() << endl;
                cout << it->toStyledString() << endl << endl;
            }
        } else
            cout << "not found." << endl;

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

    // Post processing if command complete
    if (this->state == STATE_FINISH) {
        if (this->macroState == STATE_DEF && !this->error) { // Add this entity to the index
            this->indexHandler->writeEntity(this->currEntity, this->currFields);
        } else if (this->macroState == STATE_ADD) {
            this->indexHandler->writeRelation(this->bufferEntity, this->currEntity, this->bufferValues, this->currValues);
        }

        // Cleanup
        if (this->currFields != NULL) delete this->currFields;
        if (this->currValues != NULL) delete this->currValues;
        if (this->bufferValues != NULL) delete this->bufferValues;
    }
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
    if (s.find("(") && !s.find("()")) {
        noFields = false;
        elems = this->tokenize(s, '(');
        this->currEntity = *elems.begin();
    } else
        this->currEntity = s;

    if (this->debug) {
        cout << "DEBUG -- Reading entity: " << this->currEntity << endl; // DEBUG
        if (noFields)
            cout << "DEBUG -- Entity has no fields" << endl; // DEBUG
    }
    this->fieldsProcessed = false;
    this->entityProcessed = true;

    // Process any fields
    if (!noFields)
        this->processFieldStatement(elems[1]);
}


/**
 *  Handle entity fields
 *
 *  @param string& fieldStr     Consists of one or more comma separated fields possibly terminated with ')'
 */
void Parser::processFieldStatement(const string &fieldStr) {
    std::vector<string> fields = this->tokenize(fieldStr, ',');
    std::string field;

    if (this->debug)
        cout << "DEBUG -- Reading field: " << fieldStr << endl; // DEBUG

    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;

        // Processing should be complete
        if (this->fieldsProcessed == true) {
            this->error = true;
            this->errStr = ERR_ALL_FIELDS_PROC;
            return;
        }

        // Evaluate fields
        if (field.compare(")") == 0) {
            this->fieldsProcessed = true;   // Done processing

        } else if (field.find(')') == field.length() - 1) { // e.g. <field>)
            this->fieldsProcessed = true;
            field = field.substr(0, field.length() - 1);

        } else if (field.find(')') < field.length() - 1) { // no chars after '('
            this->error = true;
            this->errStr = ERR_NO_SYM_AFTER;
            return;
        }

        // PROCESS FIELD STATEMENT
        if (this->macroState == STATE_DEF) { this->parseEntityDefinitionField(field); }
        if (this->macroState == STATE_ADD) { this->parseEntityAssignField(field); }

        // this->error = this->error || this->indexHandler->fetch(IDX_TYPE_FIELD, field);
    }
}


/**
 *  Logic for parsing entity definition statements
 *
 *  @param string& field
 */
void Parser::parseEntityDefinitionField(std::string field) {
    std::vector<string> fieldItems;
    ColumnBase* fieldType;

    fieldItems = this->tokenize(field, '_');

    if (this->debug) {
        cout << "DEBUG -- Processing field definition name: " << fieldItems[0] << endl; // DEBUG
        cout << "DEBUG -- Processing field definition type: " << fieldItems[1] << endl; // DEBUG
    }

    if (fieldItems.size() != 2) {
        this->error = true;
        this->errStr = "Invalid Entity definition format";
        return;
    }

    fieldType = getColumnType(fieldItems[1]);
    if (fieldType == NULL) {
        this->error = true;
        this->errStr = ERR_INVALID_FIELD_TYPE;
        return;
    }
    this->currFields->push_back(std::make_pair(fieldType, fieldItems[0]));
}


/**
 *  Logic for parsing entity assignment statements
 *
 *  @param string& field
 */
void Parser::parseEntityAssignField(std::string field) {
    std::vector<string> fieldItems;
    fieldItems = this->tokenize(field, '=');
    Json::Value* entityDef = this->indexHandler->fetchEntity(this->currEntity);

    // Verify that the entity has been defined
    if (entityDef == NULL) {
        this->error = true;
        this->errStr = ERR_ENT_NOT_FOUND;
        return;
    }

    // Verify that the assignment has been properly formatted
    if (fieldItems.size() != 2) {
        this->error = true;
        this->errStr = ERR_ENT_BAD_FORMAT;
        return;
    }

    // Validate Type
    if (!this->indexHandler->validateEntityFieldType(this->currEntity, fieldItems[0], fieldItems[1])) {
        this->error = true;
        this->errStr = ERR_BAD_FIELD_TYPE;
        return;
    }

    this->currValues->push_back(std::make_pair(fieldItems[0], fieldItems[1]));
}

#endif