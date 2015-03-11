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
#include "bayes.h"

#define STR_CMD_ADD "add"
#define STR_CMD_GEN "gen"
#define STR_CMD_INF "inf"
#define STR_CMD_SET "set"
#define STR_CMD_GIV "given"
#define STR_CMD_ATR "attr"
#define STR_CMD_DEF "def"
#define STR_CMD_LST "lst"
#define STR_CMD_ENT "ent"
#define STR_CMD_RM "rm"
#define STR_CMD_EXIT "exit"
#define STR_CMD_AS "as"
#define STR_CMD_REL "rel"
#define STR_CMD_FOR "for"

#define BAD_INPUT "ERR: Bad input symbol"
#define BAD_EOL "ERR: Bad end of line"
#define ERR_ENT_EXISTS "ERR: Entity already exists."
#define ERR_ENT_NOT_EXISTS "ERR: Entity not found."
#define ERR_ENT_NOT_ALPHA "ERR: Entity handles must be alpha-numeric."
#define ERR_ALL_FIELDS_PROC "ERR: All fields have already been processed."
#define ERR_NO_SYM_AFTER "ERR: No symbols permitted after ')'"
#define ERR_INVALID_FIELD_TYPE "ERR: Invalid field type"
#define ERR_ENT_BAD_FORMAT "ERR: Invalid Entity assign format"
#define ERR_ENT_FIELD_NOT_EXIST "ERR: Entity does not contain attribute"
#define ERR_BAD_FIELD_TYPE "ERR: Bad field type on instance"
#define ERR_BAD_VALUE_TYPE "ERR: Value does not belong to type"
#define ERR_INVALID_DEF_FMT "ERR: Invalid Entity definition format"
#define ERR_UNKNOWN_CMD "ERR: Unkown Command"
#define ERR_MALFORMED_CMD "ERR: Malformed Command"
#define ERR_RM_REL_CMD "ERR: Either relation not found or not successfully removed"
#define ERR_RM_ENT_CMD "ERR: Either entity not found or not successfully removed"
#define ERR_PARSE_ATTR "ERR: Could not parse entity-attribute"
#define ERR_MAL_GEN "ERR: Malformed GEN command"
#define ERR_MAL_INF "ERR: Malformed INF command"

#define WILDCARD_CHAR '*'


// STATE REPRESENTATIONS

#define STATE_START 0       // Start state

#define STATE_ADD 10        // Add a new relation
#define STATE_P0 11
#define STATE_P1 12
#define STATE_P2 13
#define STATE_P3 14

#define STATE_GEN 30        // Generate a sample entity or attribute
#define STATE_INF 70        // Infer the expected value of an attribute
#define STATE_GENINF_E1 31  // Process first entity
#define STATE_GENINF_E2 32  // Process second entity
#define STATE_GENINF_ATTR 33  // Process second entity

#define STATE_SET 80        // Generate an entity given others
#define STATE_SET_ATTR 81   // Process the attribute
#define STATE_SET_VAL 82    // Process the value

#define STATE_DEF 40        // Describes entity definitions
#define STATE_DEF_PROC 41

#define STATE_LST 50        // Lists entities or relations
#define STATE_LST_ENT 51        // Lists entities
#define STATE_LST_REL 52        // Lists relations

#define STATE_RM 60        // Remove elements
#define STATE_RM_ENT 61        // Remove entities
#define STATE_RM_REL 62        // Remove relations

#define STATE_FINISH 99     // Successful end state

#define STATE_EXIT 100       // Terminate program

using namespace std;


/**
 *  Implements an SLR parser. Valid Statements:
 *
 *      (1) ADD REL E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..])
 *      (2) GEN E1[.A_E1] GIVEN E2 [ATTR Ai=Vi[, ...]]
 *      (3) INF E1.A_E1 GIVEN E2 [ATTR Ai=Vi[, ...]]
 *      (4) DEF E1[(x1_type-x1, x2_type-x2, ...)]
 *      (5) LST REL [E1 [E2]]
 *      (6) LST ENT [E1]*
 *      (7) RM REL E1(x_1 [, x_2, ..]) E2(y_1 [, y_2, ..])
 *      (8) RM ENT [E1]*
 *      (9) SET E.A FOR E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..]) AS V *
 *
 *  (1) provides a facility for insertion into the system
 *  (2) generate a sample conditional on a set of constraints
 *  (3) infer an expected value for an attribute
 *  (4) define a new entity
 *  (5) list relations optionally dependent relational entities
 *  (6) list entities.  Either specify them or simply list all.
 *  (7) remove a relation
 *  (8) remove an entity
 *  (9) set an attribute value
 */
class Parser {

    int state;
    int macroState;

    bool entityProcessed;
    bool fieldsProcessed;
    bool parsedIDWord;      // Flag indicating whether pending ID Syntax token has been seen
    bool debug;

    bool error;
    std::string errStr;
    std::string rspStr;

    // Define lists that store state of newly defined fields and values
    defpair* currFields;
    valpair* currValues;
    valpair* bufferValues;
    std::unordered_map<std::string, std::string>* currTypes,
    std::unordered_map<std::string, std::string>* bufferTypes,

    // Define internal state that stores entity handles
    std::string currEntity;
    std::string currValue;
    std::string bufferEntity;

    // Attribute values for internal state
    std::string currAttrEntity;
    std::string bufferAttrEntity;
    std::string currAttribute;
    std::string bufferAttribute;

    // Index interface pointer
    IndexHandler* indexHandler;

    // Index interface pointer
    Bayes* bayes;

    // Parse methods
    void parseRelationPair(const std::string);
    void parseEntitySymbol(const std::string);
    void parseAttributeSymbol(const std::string, bool = false);
    void parseFieldStatement(const string &source);
    void parseEntityDefinitionField(const std::string);
    void parseEntityAssignField(const std::string);
    void parseCommaSeparatedList(const string&, const char = '=');
    void parseGenForm(const std::string, const std::string);
    void parseSet(const std::string);
    void parseValue(const std::string);

    void processGEN();
    void processINF();
    void processSET();

    void cleanup();

public:
    Parser();
    ~Parser();

    void setDebug(bool);
    void resetState();

    std::string parse(const string&);
    std::string analyze(const string&);

    std::vector<std::string> tokenize(const std::string &source, const char delimiter = ' ');
    std::vector<std::string> &tokenize(const std::string &source, const char delimiter, std::vector<std::string> &elems);
};


/**
 *  Constructor - initialize state and empty symbol table
 */
Parser::Parser() {
    this->debug = false;
    this->indexHandler = new IndexHandler();
    this->bayes = new Bayes();
    this->resetState();
}

/**
 *  Destructor
 */
Parser::~Parser() {
    delete this->bayes;
    delete this->indexHandler;
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
    this->rspStr = "";
    this->currFields = NULL;
    this->currValues = NULL;
    this->bufferValues = NULL;
    this->currTypes = NULL;
    this->bufferTypes = NULL;
    this->fieldsProcessed = false;
    this->entityProcessed = false;
    this->parsedIDWord = false;
    this->currEntity = "";
    this->currAttribute = "";
    this->currValue = "";
    this->bufferEntity = "";
    this->bufferAttribute = "";
}

/**
 *  Parse loop, calls analyzer
 */
std::string Parser::parse(const string& s) {

    vector<string> tokens = this->tokenize(s);
    std::string result;

    this->state = STATE_START;      // Initialize state
    this->error = false;            // Initialize error condition
    this->errStr = "";              // Initialize error message

    // Process command tokens
    for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
        if (this->debug)
            cout << "DEBUG -- Processing input token: " << *it << endl; // DEBUG
        result = this->analyze(*it);

        // Handle Errors detected during statement parse
        if (this->error) {
            cout << this->errStr << endl;
            return this->errStr;
        }
    }

    if (this->state == STATE_EXIT)
        exit(0);

    // If the input was not interpreted to any meaningful command
    if (this->state == STATE_START && tokens.size() > 0) {
        cout << ERR_UNKNOWN_CMD << endl;
        return ERR_UNKNOWN_CMD;
    } else if (this->state != STATE_FINISH && tokens.size() > 0) {
        cout << ERR_MALFORMED_CMD << endl;
        return ERR_MALFORMED_CMD;
    }

    return result;
}


/**
 * Lexical analyzer and state interpreter (FSM mealy model)
 */
std::string Parser::analyze(const std::string& s) {

    // Convert to lower case to enforce case insensitivity
    std::string sLower = s;
    std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);

    cout << "DEBUG -- Current state: " << this->state << endl;

    if (this->state == STATE_START) {

        if (sLower.compare(STR_CMD_ADD) == 0) {
            this->state = STATE_ADD;
            this->macroState = STATE_ADD;
        } else if (sLower.compare(STR_CMD_GEN) == 0) {
            this->state = STATE_GENINF_E1;
            this->macroState = STATE_GEN;
        } else if (sLower.compare(STR_CMD_INF) == 0) {
            this->state = STATE_GENINF_E1;
            this->macroState = STATE_INF;
        } else if (sLower.compare(STR_CMD_DEF) == 0) {
            this->state = STATE_DEF;
            this->macroState = STATE_DEF;
        } else if (sLower.compare(STR_CMD_LST) == 0) {
            this->state = STATE_LST;
            this->macroState = STATE_LST;
        } else if (sLower.compare(STR_CMD_EXIT) == 0) {
            this->state = STATE_EXIT;
        } else if (sLower.compare(STR_CMD_RM) == 0) {
            this->state = STATE_RM;
            this->macroState = STATE_RM;
        } else if (sLower.compare(STR_CMD_SET) == 0) {
            this->state = STATE_SET;
            this->macroState = STATE_SET;
        }

        cout << "DEBUG -- Setting Macro state: " << this->macroState << endl;

    } else if (this->state == STATE_ADD) {
        if (sLower.compare(STR_CMD_REL) == 0)
            this->state = STATE_P1;
        else {
            this->error = true;
            this->errStr = BAD_INPUT;
            return this->rspStr;
        }

    } else if (this->state == STATE_RM) {   // Branch to parse "RM" commands
        if (sLower.compare(STR_CMD_REL) == 0) {
            this->macroState = STATE_RM_REL;
            this->state = STATE_P1;
        } else {
            this->state = STATE_RM_ENT;
        }

    } else if (this->state == STATE_RM_ENT) {   // Branch to parse "RM ENT" commands
        this->parseEntitySymbol(sLower);
        this->state = STATE_FINISH;

    } else if (this->macroState == STATE_RM_REL && (this->state == STATE_P1 || this->state == STATE_P2)) { // Branch to parse "RM REL" commands
        this->parseRelationPair(sLower);

    } else if (this->macroState == STATE_ADD && (this->state == STATE_P1 || this->state == STATE_P2)) {
        this->parseRelationPair(sLower);

    } else if (this->macroState == STATE_GEN) {
        this->parseGenForm(sLower, ERR_MAL_GEN);

    } else if (this->macroState == STATE_INF) {
        this->parseGenForm(sLower, ERR_MAL_INF);

    } else if (this->state == STATE_DEF) {  // DEFINING new entities

        this->currFields = new vector<std::pair<ColumnBase, std::string>>;
        this->state == STATE_DEF_PROC;
        this->parseEntitySymbol(sLower);

        // Validate that entity is alpha-numeric
        boost::regex e("^[a-zA-Z0-9]*$");
        if (!boost::regex_match(this->currEntity.c_str(), e)) {
            this->error = true;
            this->errStr = ERR_ENT_EXISTS;
            return this->rspStr;
        }

        // Ensure this entity has not already been defined
        if (this->indexHandler->existsEntity(this->currEntity)) {
            this->error = true;
            this->errStr = ERR_ENT_EXISTS;
            return this->rspStr;
        }

        if (this->fieldsProcessed)
            this->state = STATE_FINISH;

    } else if (this->state == STATE_DEF_PROC) {
        this->parseFieldStatement(sLower);
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

    } else if (this->macroState == STATE_SET) {
        this->parseSet(sLower);

    } else if (this->state == STATE_FINISH) {  // Ensure processing is complete - no symbols should be left at this point
        this->error = true;
        this->errStr = BAD_EOL;
        return this->rspStr;
    }

    // Post processing if command complete
    if (this->state == STATE_FINISH) {

        if (this->debug) {
            cout << "DEBUG -- Finishing statement processing." << endl;
            cout << "DEBUG -- Macro state: " << this->macroState << endl;
            cout << "DEBUG -- Error state: " << this->error << endl;
        }

        // If there's an error cleanup and bail
        if (this->error) { this->cleanup(); return this->rspStr; }

        if (this->macroState == STATE_DEF && !this->error) { // Add this entity to the index
            Entity e(this->currEntity, *(this->currFields));
            this->indexHandler->writeEntity(e);
            this->rspStr = "Entity successfully added";

            if (this->debug)
                cout << "DEBUG -- Writing definition of entity." << endl;

        } else if (this->macroState == STATE_ADD) {
            Relation r(this->bufferEntity, this->currEntity, *(this->bufferValues), *(this->currValues), *(this->bufferTypes), *(this->currTypes));
            this->indexHandler->writeRelation(r);
            this->rspStr = "Relation successfully added";

            if (this->debug)
                cout << "DEBUG -- Adding relation." << endl;

        } else if (this->macroState == STATE_GEN) {
            this->processGEN();

        } else if (this->macroState == STATE_INF) {
            this->processINF();

        } else if (this->macroState == STATE_LST_ENT) {

            std::vector<Json::Value> entities;
            cout << "Current Matched Entities for \"" << this->currEntity << "\""<< endl;
            entities = this->indexHandler->fetchPatternJson(this->indexHandler->generateEntityKey(this->currEntity));
            if (entities.size() != 0)
                for (std::vector<Json::Value>::iterator it = entities.begin() ; it != entities.end(); ++it)
                    this->rspStr += std::string((*it).toStyledString());
            else
                this->rspStr = "Not found";
            cout << this->rspStr << endl;

        } else if (this->macroState == STATE_LST_REL) {
            // Get all relations on given entities
            cout << "Current Matched Relations for \"" << this->bufferEntity << "\" and \"" << this->currEntity << "\"" << endl;

            // Combine buffer and current values
            for (std::vector<std::pair<std::string, std::string>>::iterator it = this->bufferValues->begin() ; it != bufferValues->end(); ++it)
                this->currValues->push_back(*it);

            // Fetch relations and filter on attribute criteria
            std::vector<Json::Value> relationsJson = this->indexHandler->fetchRelationPrefix(this->bufferEntity, this->currEntity);
            std::vector<Relation> relations = this->indexHandler->Json2RelationVector(relationsJson);
            AttributeBucket ab = AttributeBucket(this->currEntity, *(this->currValues));
            this->indexHandler->filterRelations(relations, ab);

            // for each relation determine if they match the condition criteria
            for (std::vector<Relation>::iterator it = relations.begin() ; it != relations.end(); ++it)
                this->rspStr += std::string(it->toJson().toStyledString());
            cout << this->rspStr << endl;

        } else if (this->macroState == STATE_RM_REL) {
            // Handle the logic for the removal of matching relations
            Relation r(this->bufferEntity, this->currEntity, *(this->bufferValues), *(this->currValues), *(this->bufferTypes), *(this->currTypes));
            if (this->indexHandler->removeRelation(r)) {
                this->rspStr = "Relation removed";
            } else {
                this->rspStr = ERR_RM_REL_CMD;
            }
            cout << this->rspStr << endl;

        } else if (this->macroState == STATE_RM) {
            // Handle the logic for the removal of matching entities
            if (this->indexHandler->removeEntity(this->currEntity)) {
                this->rspStr = "Entity removed";
            } else {
                this->rspStr = ERR_RM_ENT_CMD;
            }
            cout << this->rspStr << endl;
        } else if (this->macroState == STATE_SET) {
            this->processSET();
        }

        // Cleanup
        this->cleanup();
    }

    return this->rspStr;
}


/**
 * Performs cleanup of state containers after statement processing
 */
void Parser::cleanup() {
    if (this->debug)
        cout << "DEBUG -- cleanup. freeing out statement allocation." << endl;
    if (this->currFields != NULL) { delete this->currFields; this->currFields = NULL; }
    if (this->currValues != NULL) { delete this->currValues; this->currValues = NULL; }
    if (this->bufferValues != NULL) { delete this->bufferValues; this->bufferValues = NULL; }
    if (this->currTypes != NULL) { delete this->currTypes; this->currTypes = NULL; }
    if (this->bufferTypes != NULL) { delete this->bufferTypes; this->bufferTypes = NULL; }
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
void Parser::parseEntitySymbol(const std::string s) {

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
        this->parseFieldStatement(elems[1]);
}

/**
 *  Parses a single attribute value of the form <entity>.<attribute>
 *  Allow read of entity only to be enforced
 *
 *  @param s    input string - e.g. "car.wheels"
 */
void Parser::parseAttributeSymbol(const std::string s, bool entityOnly) {

    // Tokenize the string
    std::vector<std::string> elems;
    elems = this->tokenize(s, '.');

    // Ensure that there are two tokens
    if (elems.size() == 2) {
        this->currAttrEntity = elems[0];
        this->currAttribute = elems[1];
    } else if (entityOnly && elems.size() == 1) {
        this->currAttrEntity = elems[0];
    } else {
        this->error = true;
        this->errStr = ERR_PARSE_ATTR;
    }

    // Debug output
    if (this->debug)
        if (entityOnly)
            cout << "DEBUG -- Reading entity/attribute: " << this->currAttrEntity << endl; // DEBUG
        else
            cout << "DEBUG -- Reading entity/attribute: " << this->currAttrEntity << "." << this->currAttribute << endl; // DEBUG

    this->entityProcessed = true;
}

/**
 *  Fetches an attribute value from the input
 *
 *  @param s    input string - e.g. 55, "hello", 2.1
 */
void Parser::parseValue(const std::string s) {
    // Validate Field type
    if (!this->indexHandler->validateEntityFieldType(this->currEntity, this->currAttribute, s)) {
        this->error = true;
        this->errStr = ERR_BAD_VALUE_TYPE;
    }
    this->currValue = s;
}

/**
 *  Handle entity fields
 *
 *  @param string& fieldStr     Consists of one or more comma separated fields possibly terminated with ')'
 */
void Parser::parseFieldStatement(const std::string &fieldStr) {
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
 *  Parses a comma seperated list of attribute value pairs
 *  Default is something like a_1=v_1,a_2=v_2,...,a_n=v_n
 *
 *  @param string& fieldStr     string storing a comma seperated list
 */
void Parser::parseCommaSeparatedList(const string &fieldStr, const char fieldDelimiter) {
    std::vector<string> fields = this->tokenize(fieldStr, ',');
    std::string field;

    if (this->debug)
        cout << "DEBUG -- Reading field: " << fieldStr << endl; // DEBUG

    for (std::vector<string>::iterator it = fields.begin() ; it != fields.end(); ++it) {
        field = *it;

        // On Assignment
        if (fieldDelimiter == '=')
            this->parseEntityAssignField(field);
    }
    this->fieldsProcessed == true;
}

/**
 *  Logic for parsing entity definition statements
 *
 *  @param string& field
 */
void Parser::parseEntityDefinitionField(const std::string field) {
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
    if (std::strcmp(fieldType.getType(), COLTYPE_NAME_NULL) == 0 {
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
void Parser::parseEntityAssignField(const std::string field) {
    std::vector<std::string> fieldItems;
    fieldItems = this->tokenize(field, '=');

    // Verify that the entity has been defined
    if (!this->indexHandler->existsEntity(this->currEntity)) {
        this->error = true;
        this->errStr = std::string(ERR_ENT_NOT_EXISTS) + std::string(" -> \"") + this->currEntity + std::string("\"");
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

    // Push the current value
    this->currValues->push_back(std::make_pair(fieldItems[0], fieldItems[1]));

    // Fetch the field type from the entity - it must not be null type
    std::string type = this->indexHandler->fetchEntityFieldType(this->currEntity, fieldItems[0]);
    if (std::strcmp(type, COLTYPE_NAME_NULL) != 0)
        // Push the current field type
        this->currTypes->insert(std::make_pair(fieldItems[0], type));
    else {
        this->error = true;
        this->errStr = ERR_BAD_FIELD_TYPE;
        return;
    }
}


/**
 *  Stateless method for parsing a pair of entity descriptors defining a relation
 */
void Parser::parseRelationPair(const std::string symbol) {
    // Determine if entity or fields need to be processed
    if (this->entityProcessed) {
        this->parseFieldStatement(symbol);
    } else {
        if (this->currValues != NULL) delete this->currValues;
        if (this->currTypes != NULL) delete this->currTypes;
        this->currValues = new vector<std::pair<std::string, std::string>>;
        this->currTypes = new std::unordered_map<std::string, std::string>;
        this->parseEntitySymbol(symbol);

        // Ensure that entities exist if we are adding a new relation
        if (this->macroState == STATE_ADD)
            if (!this->indexHandler->existsEntity(this->currEntity)) {
                this->error = true;
                this->errStr = std::string(ERR_ENT_NOT_EXISTS) + std::string(" -> \"") + this->currEntity +std::string("\"");
                return;
            }
    }

    // If all fields have been processed transition
    if (this->fieldsProcessed)
        if (this->state == STATE_P1) {
            this->state = STATE_P2;
            this->bufferEntity = this->currEntity;
            this->bufferValues = this->currValues;
            this->bufferTypes = this->currTypes;
            this->currValues = new vector<std::pair<std::string, std::string>>;
            this->currTypes = new std::unordered_map<std::string, std::string>;
            this->fieldsProcessed = false;
            this->entityProcessed = false;

        } else if (this->state == STATE_P2) {
            this->state = STATE_FINISH;
        }
}

/**
 *  Stateless method for parsing GEN or INF Commands
 *
 *  SYNTAX: GEN E1[.A_E1] GIVEN E2 [ATTR Ai=Vi[, ...]]
 *          INF E1.A_E1 GIVEN E2 [ATTR Ai=Vi[, ...]]
 */
void Parser::parseGenForm(const std::string inputToken, const std::string err) {
    switch (this->state) {
        case STATE_GENINF_E1:   // if E1 parse the first entity
            this->parseAttributeSymbol(inputToken);
            this->state = STATE_GENINF_E2;
            this->parsedIDWord = false;
            break;

        case STATE_GENINF_E2:  // if E2 parse the first entity
            if (inputToken.compare(STR_CMD_GIV) == 0 && !this->parsedIDWord) {
                this->parsedIDWord = true;
                break;
            } else if (inputToken.compare(STR_CMD_GIV) == 0) {
                this->error = true;
                this->errStr = err;
            }

            // Store the target entity and attribute
            this->bufferAttrEntity = this->currAttrEntity;
            this->bufferAttribute = this->currAttribute;

            this->parseAttributeSymbol(inputToken, true);
            this->currEntity = this->currAttrEntity;
            this->state = STATE_GENINF_ATTR;
            this->parsedIDWord = false;
            break;

        case STATE_GENINF_ATTR: // if ATTR parse the first entity

            // Initialize current value list
            if (this->currValues != NULL) delete this->currValues;
            this->currValues = new vector<std::pair<std::string, std::string>>;

            if (inputToken.compare(STR_CMD_ATR) == 0 && !this->parsedIDWord) {
                this->parsedIDWord = true;
                break;
            } else if (inputToken.compare(STR_CMD_ATR) == 0) {
                this->error = true;
                this->errStr = err;
            }
            this->state = STATE_FINISH;
            this->parseCommaSeparatedList(inputToken);
            this->parsedIDWord = false;
            break;

        default:
            this->error = true;
            this->errStr = err;
    }
}

/**
 *  Stateless method for parsing SET Command
 *
 *  SYNTAX: SET E.A FOR E1(x1=vx1[, x2=vx2, ..]) E2(y1=vy1[, y2=vy2, ..]) AS V *
 */
void Parser::parseSet(const std::string inputToken) {
    switch (this->state) {
        case STATE_P0:  // Parse entity/attribute to set
            this->parseAttributeSymbol(inputToken);
            this->state = STATE_P1;
            break;
        case STATE_P1:   // Parse first entity attribute settings
            if (inputToken.compare(STR_CMD_FOR) == 0) break;
            this->parseRelationPair(inputToken);    // handles transition to P2
            break;
        case STATE_P2:   // Parse second entity attribute settings
            this->parseRelationPair(inputToken);
            this->state = STATE_P3;
            break;
        case STATE_P3:  // Parse value to set
            if (inputToken.compare(STR_CMD_AS) == 0) break;
            this->parseValue(inputToken);
            this->state = STATE_FINISH;
            break;
    }
}

/**
 *  Methods to process commands to be executed after successful parse
 */

void Parser::processGEN() {
    // Construct attribute bucket
    AttributeBucket ab;
    ab.addAttributes(this->currAttrEntity, *(this->currValues));

    // Call sampling method from Bayes for relations
    Relation r = this->bayes->samplePairwise(this->bufferAttrEntity, this->currEntity, ab);

    // Print the sample
    cout << r.toJson().toStyledString() << endl;
}

void Parser::processINF() {

    // Construct attribute bucket
    AttributeBucket ab;
    ab.addAttributes(this->currAttrEntity, *(this->currValues));

    // Call sampling method from Bayes for relations
    AttributeTuple* at = new AttributeTuple(this->bufferAttrEntity, this->bufferAttribute, "");
    float exp = this->bayes->expectedAttribute(*at, ab);

    // Print the expected value
    cout << exp << endl;
    delete at;
}

void Parser::processSET() {

    // FIRST DETERMINE RELATIONS TO WHICH THIS APPLIES

    // Construct attribute bucket
    AttributeBucket ab;
    ab.addAttributes(this->currAttrEntity, *(this->currValues));

    std::vector<Json::Value> relations = this->indexHandler->fetchRelationPrefix(this->bufferAttrEntity, this->currAttrEntity);
    this->indexHandler->filterRelations(relations, ab);

    // Call sampling method from Bayes for relations
    for (std::vector<Json::Value>::iterator it = relations.begin() ; it != relations.end(); ++it) {
        // Each iteration should be atomic
        this->indexHandler->removeRelation(*it);
        if (this->bufferAttrEntity.compare((*it)[JSON_ATTR_REL_ENTL].asCString()) == 0) {
            if (!(*it)[JSON_ATTR_REL_FIELDSL].isMember(this->bufferAttribute)) {
                // TODO - handle error mode
                cout << "Attribute not found in SET: " << this->bufferAttrEntity << "." << this->bufferAttribute << endl; // DEBUG
                continue;
            }
            // TODO - check value against attribute type
            (*it)[JSON_ATTR_REL_FIELDSL][this->bufferAttribute] = this->currValue;
        } else if (this->bufferAttrEntity.compare((*it)[JSON_ATTR_REL_ENTR].asCString()) == 0) {
            if (!(*it)[JSON_ATTR_REL_FIELDSR].isMember(this->bufferAttribute)) {
                // TODO - handle error mode
                cout << "Attribute not found in SET: " << this->bufferAttrEntity << "." << this->bufferAttribute << endl; // DEBUG
                continue;
            }
            // TODO - check value against attribute type
            (*it)[JSON_ATTR_REL_FIELDSR][this->bufferAttribute] = this->currValue;
        }
        this->indexHandler->writeRelation(*it);
        cout << "DEBUG -- SET attribute: " << this->bufferAttrEntity << "." << this->bufferAttribute << " to " << this->currValue << endl; // DEBUG
    }
}

#endif