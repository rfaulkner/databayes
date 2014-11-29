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
#define STR_CMD_GEN "gen"
#define STR_CMD_CON "constrain"
#define STR_CMD_REL "rel"
#define STR_CMD_DEF "def"
#define STR_CMD_LST "lst"
#define STR_CMD_ENT "ent"
#define STR_CMD_EXIT "exit"

#define BAD_INPUT "ERR: Bad input symbol"
#define BAD_EOL "ERR: Bad end of line"
#define ERR_ENT_EXISTS "ERR: Entity already exists."
#define ERR_ENT_NOT_EXISTS "ERR: Entity not found."
#define ERR_ALL_FIELDS_PROC "ERR: All fields have already been processed."
#define ERR_NO_SYM_AFTER "ERR: No symbols permitted after ')'"
#define ERR_INVALID_FIELD_TYPE "ERR: Invalid field type"
#define ERR_ENT_BAD_FORMAT "ERR: Invalid Entity assign format"
#define ERR_ENT_FIELD_NOT_EXIST "ERR: Entity does not contain attribute"
#define ERR_BAD_FIELD_TYPE "ERR: Bad field type on instance"
#define ERR_INVALID_DEF_FMT "ERR: Invalid Entity definition format"
#define ERR_UNKNOWN_CMD "ERR: Unkown Command"
#define ERR_MALFORMED_CMD "ERR: Malformed Command"


#define WILDCARD_CHAR '*'


// STATE REPRESENTATIONS

#define STATE_START 0       // Start state

#define STATE_ADD 10        // Add a new relation
#define STATE_P1 12
#define STATE_P2 14

#define STATE_GEN 30        // Generate an entity given others
#define STATE_GEN_REL 31

#define STATE_DEF 40        // Describes entity definitions
#define STATE_DEF_PROC 41

#define STATE_LST 50        // Lists entities or relations
#define STATE_LST_ENT 51        // Lists entities
#define STATE_LST_REL 52        // Lists relations

#define STATE_FINISH 99     // Successful end state

#define STATE_EXIT 100       // Terminate program

using namespace std;


/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (2) GEN REL E1[(x_1=v_1, x_2=v_2, ...)] CONSTRAIN E2[, E3, ...]
 *      (3) DEF E1[(x_1, x_2, ...)]
 *      (4) LST REL [E1 [E2]]
 *      (5) LST ENT [E1]*
 *
 *  (1) provides a facility for insertion into the system
 *  (2) generate a sample conditional on an entity
 *  (3) define a new entity
 *  (4) list relations optionally dependent relational entities
 *  (5) list entities.  Either specify them or simply list all.
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
    void parseRelationPair(std::string);
    void parseEntitySymbol(std::string);
    void processFieldStatement(const string &source);
    void parseEntityDefinitionField(std::string);
    void parseEntityAssignField(std::string);
    void cleanup();

public:
    Parser();

    void setDebug(bool);
    void resetState();

    bool parse(const string&);
    void analyze(const string&);

    std::vector<std::string> tokenize(const std::string &source, const char delimiter = ' ');
    std::vector<std::string> &tokenize(const std::string &source, const char delimiter, std::vector<std::string> &elems);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->debug = false;
    this->indexHandler = new IndexHandler();
    this->resetState();
}


/**
 * Setter for debug property
 */
void Parser::setDebug(bool debugVal) {
    this->debug = debugVal;
}

/**
 * Resets the parser state
 */
void Parser::resetState() {
    this->state = STATE_START;
    this->error = false;
    this->errStr = "";
    this->currFields = NULL;
    this->currValues = NULL;
    this->bufferValues = NULL;
    this->fieldsProcessed = false;
    this->entityProcessed = false;
    this->currEntity = "";
    this->bufferEntity = "";
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

        // Handle Errors detected during statement parse
        if (this->error) {
            cout << this->errStr << endl;
            return false;
        }
    }

    if (this->state == STATE_EXIT)
        exit(0);

    // If the input was not interpreted to any meaningful command
    if (this->state == STATE_START && tokens.size() > 0) {
        cout << ERR_UNKNOWN_CMD << endl;
        return false;
    } else if (this->state != STATE_FINISH && tokens.size() > 0) {
        cout << ERR_MALFORMED_CMD << endl;
        return false;
    }

    return true;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
void Parser::analyze(const std::string& s) {

    // Convert to lower case to enforce case insensitivity
    string sLower = s;
    std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);

    cout << "DEBUG -- Current state: " << this->state << endl;

    if (this->state == STATE_START) {

        if (sLower.compare(STR_CMD_ADD) == 0) {
            this->state = STATE_ADD;
            this->macroState = STATE_ADD;
        } else if (sLower.compare(STR_CMD_GEN) == 0) {
            this->state = STATE_GEN;
            this->macroState = STATE_GEN;
        } else if (sLower.compare(STR_CMD_DEF) == 0) {
            this->state = STATE_DEF;
            this->macroState = STATE_DEF;
        } else if (sLower.compare(STR_CMD_LST) == 0) {
            this->state = STATE_LST;
            this->macroState = STATE_LST;
        } else if (sLower.compare(STR_CMD_EXIT) == 0) {
            this->state = STATE_EXIT;
        }

        cout << "DEBUG -- Setting Macro state: " << this->macroState << endl;

    } else if (this->state == STATE_ADD || this->state == STATE_GEN) {
        if (sLower.compare(STR_CMD_REL) == 0)
            switch (this->state) {
            case STATE_ADD:
                this->state = STATE_P1;
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

    } else if (this->macroState == STATE_ADD && (this->state == STATE_P1 || this->state == STATE_P2)) {
        this->parseRelationPair(sLower);

    } else if (this->state == STATE_GEN_REL && (this->state == STATE_P1 || this->state == STATE_P2)) {
        this->parseRelationPair(sLower);

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
        if (sLower.compare(STR_CMD_REL) == 0) {
            this->macroState = STATE_LST_REL;
            this->state = STATE_P1;
        } else if (sLower.compare(STR_CMD_ENT) == 0)
            this->state = STATE_LST_ENT;

    } else if (this->state == STATE_LST_ENT) {

        this->macroState = STATE_LST_ENT;
        this->parseEntitySymbol(sLower);
        this->state = STATE_FINISH;

    } else if (this->macroState == STATE_LST_REL && (this->state == STATE_P1 || this->state == STATE_P2)) {
        this->parseRelationPair(sLower);

    } else if (this->state == STATE_FINISH) {  // Ensure processing is complete - no symbols should be left at this point
        this->error = true;
        this->errStr = BAD_EOL;
        return;
    }

    // Post processing if command complete
    if (this->state == STATE_FINISH) {

        if (this->debug) {
            cout << "DEBUG -- Finishing statement processing." << endl;
            cout << "DEBUG -- Macro state: " << this->macroState << endl;
            cout << "DEBUG -- Error state: " << this->error << endl;
        }

        // If there's an error cleanup and bail
        if (this->error) { this->cleanup(); return; }

        if (this->macroState == STATE_DEF && !this->error) { // Add this entity to the index
            this->indexHandler->writeEntity(this->currEntity, *(this->currFields));

            if (this->debug)
                cout << "DEBUG -- Writing definition of entity." << endl;

        } else if (this->macroState == STATE_ADD) {
            this->indexHandler->writeRelation(this->bufferEntity, this->currEntity, *(this->bufferValues), *(this->currValues));
            if (this->debug)
                cout << "DEBUG -- Adding relation." << endl;

        } else if (this->macroState == STATE_GEN_REL) {
            // TODO - Add logic to sample relation

        } else if (this->macroState == STATE_LST_ENT) {

            std::vector<Json::Value>* entities;
            cout << "Current Matched Entities for \"" << this->currEntity << "\""<< endl;
            entities = this->indexHandler->fetchPatternJson(this->indexHandler->generateEntityKey(this->currEntity));
            if (entities != NULL)
                for (std::vector<Json::Value>::iterator it = entities->begin() ; it != entities->end(); ++it)
                    cout << (*it).toStyledString() << endl << endl;
            else
                cout << "not found." << endl;

        } else if (this->macroState == STATE_LST_REL) {
            // TODO - logic to extract matching entities

            // Get all relations on given entities
            cout << "Current Matched Relations for \"" << this->bufferEntity << "\" and \"" << this->currEntity << "\"" << endl;
            std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(this->bufferEntity, this->currEntity);

            // for each relation determine if they match the condition criteria
            for (std::vector<Json::Value>::iterator it = relations.begin() ; it != relations.end(); ++it)
                cout << (*it).toStyledString() << endl << endl;

            // print out results

        }

        // Cleanup
        this->cleanup();
    }
}


/**
 * Performs cleanup of state containers after statement processing
 */
void Parser::cleanup() {
    if (this->currFields != NULL) { delete this->currFields; this->currFields = NULL; }
    if (this->currValues != NULL) { delete this->currValues; this->currValues = NULL; }
    if (this->bufferValues != NULL) { delete this->bufferValues; this->bufferValues = NULL; }
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
    this->fieldsProcessed = false;
    if (s.find("(") != std::string::npos && strstr(s.c_str(), "()") == NULL) {
        noFields = false;
        elems = this->tokenize(s, '(');
        this->currEntity = *elems.begin();

    } else {
        this->currEntity = s;
        this->fieldsProcessed = true;
    }

    if (this->debug) {
        cout << "DEBUG -- Reading entity: " << this->currEntity << endl; // DEBUG
        if (noFields)
            cout << "DEBUG -- Entity has no fields" << endl; // DEBUG
    }

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
        this->errStr = ERR_INVALID_DEF_FMT;
        return;
    }

    fieldType = getColumnType(fieldItems[1]);
    if (!fieldType) {
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
    std::vector<std::string> fieldItems;
    fieldItems = this->tokenize(field, '=');

    // Verify that the entity has been defined
    if (!this->indexHandler->existsEntity(this->currEntity)) {
        this->error = true;
        this->errStr = ERR_ENT_NOT_EXISTS;
        return;
    }

    // Verify that the assignment has been properly formatted
    if (fieldItems.size() != 2) {
        this->error = true;
        this->errStr = ERR_ENT_BAD_FORMAT;
        return;
    }

    // Verify that the entity contains this attribute
    if (!this->indexHandler->existsEntityField(this->currEntity, fieldItems[0])) {
        this->error = true;
        this->errStr = ERR_ENT_FIELD_NOT_EXIST;
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


/**
 *  Stateless method for parsing a pair of entity descriptors defining a relation
 */
void Parser::parseRelationPair(std::string symbol) {
    // Determine if entity or fields need to be processed
    if (this->entityProcessed) {
        this->processFieldStatement(symbol);
    } else {
        if (this->currValues != NULL) delete this->currValues;
        this->currValues = new vector<std::pair<std::string, std::string>>;
        this->parseEntitySymbol(symbol);

        // Ensure that entities exist
        if (!this->indexHandler->existsEntity(this->currEntity)) {
            this->error = true;
            this->errStr = ERR_ENT_NOT_EXISTS;
            return;
        }
    }

    // If all fields have been processed transition
    if (this->fieldsProcessed)
        if (this->state == STATE_P1) {
            this->state = STATE_P2;
            this->bufferEntity = this->currEntity;
            this->bufferValues = this->currValues;
            this->currValues = new vector<std::pair<std::string, std::string>>;
            this->fieldsProcessed = false;
            this->entityProcessed = false;

        } else if (this->state == STATE_P2) {
            this->state = STATE_FINISH;
        }
}

#endif