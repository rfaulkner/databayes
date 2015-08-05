
/*
 *  client.cpp
 *
 *  Handles the client env.
 *
 *  Created by Ryan Faulkner on 2014-06-08
 *  Copyright (c) 2014. All rights reserved.
 */

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <assert.h>
#include <json/json.h>

#include "column_types.h"
#include "redis.h"
#include "md5.h"
#include "index.h"
#include "bayes.h"
#include "models/models.h"

#define REDISHOST "127.0.0.1"
#define REDISPORT 6379

using namespace std;

typedef void (*FnPtr)();
std::unordered_map<std::string, std::pair<bool, FnPtr>> tests =
    std::unordered_map<std::string, std::pair<bool, FnPtr>>();

// Global vectors to manage created entities and relations
std::vector<Entity> openEntities;
std::vector<Relation> openRelations;


/** Create an entity for testing */
void makeTestEntity(std::string name, defpair cols) {
    Entity e(name, cols);
    openEntities.push_back(e);
}


/** Create a relations for testing */
void makeTestRelation(std::string e1,
                      std::string e2,
                      valpair e1Vals,
                      valpair e2Vals,
                      std::unordered_map<std::string, std::string> e1Types,
                      std::unordered_map<std::string, std::string> e2Types) {
    Relation r(e1, e2, e1Vals, e2Vals, e1Types, e2Types);
    openRelations.push_back(r);
}

/** Free the allocated entities and relations */
void releaseObjects() {
    IndexHandler ih;
    for (std::vector<Entity>::iterator it = openEntities.begin();
            it != openEntities.end(); ++it)
        ih.removeEntity(*it);
    for (std::vector<Relation>::iterator it = openRelations.begin();
            it != openRelations.end(); ++it)
        ih.removeRelation(*it);
    openEntities.clear();
    openRelations.clear();
}

// TODO - change the key to be properly descriptive

/** Performs a write over the test entity set */
void writeEntities() {
    IndexHandler ih;
    RedisHandler rds(REDISDBTEST, REDISPORT);
    for (std::vector<Entity>::iterator it = openEntities.begin();
            it != openEntities.end(); ++it)
        it->write(rds);
}

void removeEntities() {
    IndexHandler ih;
    RedisHandler rds(REDISDBTEST, REDISPORT);
    for (std::vector<Entity>::iterator it = openEntities.begin();
            it != openEntities.end(); ++it)
        it->remove(rds);
}

void writeRelations() {
    IndexHandler ih;
    RedisHandler rds(REDISDBTEST, REDISPORT);
    for (std::vector<Relation>::iterator it = openRelations.begin();
            it != openRelations.end(); ++it)
        it->write(rds);
}

void removeRelations() {
    IndexHandler ih;
    RedisHandler rds(REDISDBTEST, REDISPORT);
    for (std::vector<Relation>::iterator it = openRelations.begin();
            it != openRelations.end(); ++it)
        it->remove(rds);
}


/* -- SETUP FUNCTIONS -- */


/*  Builds a vector of relations */
void setRelationsList1() {

    // Initialize fields and types
    defpair fields_ent_1, fields_ent_2;
    valpair fields_rel_1, fields_rel_2, fields_rel_3;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    ColumnBase* intCol = new IntegerColumn();
    ColumnBase* strCol = new StringColumn();

    fields_ent_1.push_back(std::make_pair(intCol, "a"));
    fields_ent_2.push_back(std::make_pair(strCol, "b"));

    fields_rel_1.push_back(std::make_pair("a", "1"));
    fields_rel_2.push_back(std::make_pair("b", "hello"));
    fields_rel_3.push_back(std::make_pair("b", "goodbye"));

    types_rel_1.insert(std::make_pair("a", COLTYPE_NAME_INT));
    types_rel_2.insert(std::make_pair("b", COLTYPE_NAME_STR));

    // Initialize entities
    makeTestEntity("_x", fields_ent_1);
    makeTestEntity("_y", fields_ent_2);

    // Add relation set
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_2,
        types_rel_1, types_rel_2);
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_3,
        types_rel_1, types_rel_2);

    delete intCol;
    delete strCol;
}

/* Builds a vector of relations */
void setRelationsList2() {

    // Initialize fields and types
    defpair fields_ent_1, fields_ent_2;
    valpair fields_rel_1, fields_rel_2, fields_rel_3, fields_rel_4;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    ColumnBase* intCol = new IntegerColumn();
    ColumnBase* floatCol = new StringColumn();

    fields_ent_1.push_back(std::make_pair(intCol, "a"));
    fields_ent_2.push_back(std::make_pair(floatCol, "b"));

    fields_rel_1.push_back(std::make_pair("a", "1"));
    fields_rel_2.push_back(std::make_pair("b", "2.0"));
    fields_rel_3.push_back(std::make_pair("a", "11"));
    fields_rel_4.push_back(std::make_pair("b", "12.0"));

    types_rel_1.insert(std::make_pair("a", COLTYPE_NAME_INT));
    types_rel_2.insert(std::make_pair("b", COLTYPE_NAME_FLOAT));

    // Initialize entities
    makeTestEntity("_x", fields_ent_1);
    makeTestEntity("_y", fields_ent_2);

    // Add relation set
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_2,
        types_rel_1, types_rel_2);
    makeTestRelation("_x", "_y", fields_rel_3, fields_rel_4,
        types_rel_1, types_rel_2);

    delete intCol;
    delete floatCol;
}

/* -- TEST FUNCTIONS -- */

/** Test to ensure that redis keys are correctly returned */
void testRedisSetGetRemove() {
    RedisHandler r(REDISDBTEST, REDISPORT);
    r.connect();
    r.write("foo", "bar");
    assert(r.read("foo").compare("bar") == 0);
    r.deleteKey("foo");
    assert(!r.exists("foo"));
}


/** Test to ensure that redis keys are correctly returned */
void testRedisKeys() {
    RedisHandler r(REDISDBTEST, REDISPORT);
    std::vector<std::string> vec;

    r.connect();
    r.write("foo1", "bar");
    r.write("foo2", "bar");
    r.write("foo3", "bar");
    vec = r.keys("foo*");
    for (std::vector<std::string>::iterator it = vec.begin();
            it != vec.end(); ++it) {
        assert(it->compare("foo1") == 0 ||
            it->compare("foo2") == 0 ||
            it->compare("foo3") == 0
        );
        r.deleteKey(*it);
    }
}


/** Test to ensure that md5 hashing works */
void testMd5Hashing() {
    assert(std::string("mykey").compare(md5("mykey")) != 0);
}

/** Test to ensure that md5 hashing works */
void testRegexForTypes() {
    IntegerColumn ic;
    FloatColumn fc;
    assert(ic.validate("1981"));
    assert(fc.validate("5.2"));
}


/** Test to ensure that md5 hashing works */
void testOrderPairAlphaNumeric() {
    IndexHandler ih;
    assert(
        std::strcmp(ih.orderPairAlphaNumeric("b", "a").c_str(),
            ("a" + std::string(KEY_DELIMETER) + "b").c_str()) == 0);
}

/**
 * Test to ensure that relation entities are encoded properly
 */
void testJSONEntityEncoding() {

    IndexHandler ih;
    Json::Value json;
    defpair fields_ent;
    ColumnBase* col =  new IntegerColumn();

    // Create fields
    fields_ent.push_back(std::make_pair(col, "a"));
    Entity e("test", fields_ent);
    ih.writeEntity(e);     // Create the entity
    ih.fetchEntity("test", json);           // Fetch the entity representation

    // Assert that entity as read matches definition
    assert(std::strcmp(json["entity"].asCString(), "test") == 0 &&
            std::strcmp(json["fields"]["a"].asCString(), "integer") == 0 &&
            json["fields"]["_itemcount"].asInt() == 1
    );
    ih.removeEntity("test");                // Remove the entity
    delete col;
}


/**
 * Test to ensure that relation fields are encoded properly
 */
void testJSONRelationEncoding() {
    IndexHandler ih;
    std::vector<Json::Value> ret;
    defpair fields_ent_1;
    defpair fields_ent_2;
    valpair fields_rel_1;
    valpair fields_rel_2;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    // Popualate fields
    ColumnBase* intCol = new IntegerColumn();
    ColumnBase* strCol = new StringColumn();
    fields_ent_1.push_back(std::make_pair(intCol, "a"));
    fields_ent_2.push_back(std::make_pair(strCol, "b"));
    fields_rel_1.push_back(std::make_pair("a", "1"));
    fields_rel_2.push_back(std::make_pair("b", "hello"));
    types_rel_1.insert(std::make_pair("a", COLTYPE_NAME_INT));
    types_rel_2.insert(std::make_pair("b", COLTYPE_NAME_FLOAT));

    // Create entities
    Entity e1("test_1", fields_ent_1), e2("test_2", fields_ent_2);
    ih.writeEntity(e1);
    ih.writeEntity(e2);

    // Create relation in redis
    Relation r("test_1", "test_2", fields_rel_1, fields_rel_2,
        types_rel_1, types_rel_2);
    ih.writeRelation(r);

    // Fetch the entity representation
    ret = ih.fetchRelationPrefix("test_1", "test_2");
    cout << "TEST RELATION:" << endl << endl << ret[0].toStyledString() << endl;

    // Assert that entity as read matches definition
    assert(
        std::strcmp(ret[0]["entity_left"].asCString(), "test_1") == 0 &&
        std::strcmp(ret[0]["entity_right"].asCString(), "test_2") == 0 &&
        std::strcmp(ret[0]["fields_left"]["a"].asCString(), "1") == 0 &&
        std::strcmp(ret[0]["fields_right"]["b"].asCString(), "hello") == 0 &&
        ret[0]["fields_left"]["_itemcount"].asInt() == 1 &&
        ret[0]["fields_right"]["_itemcount"].asInt() == 1
    );
    ih.removeEntity("test_1");                // Remove the entity
    ih.removeEntity("test_2");                // Remove the entity
    ih.removeRelation(r);    // Remove the relation

    delete intCol;
    delete strCol;
}


/**
 *  Tests that parseEntityAssignField correctly flags invalid assignments
 *  to integer fields
 */
void testFieldAssignTypeMismatchInteger() {
    IndexHandler ih;
    Json::Value json;
    defpair fields_ent;

    // Create fields
    ColumnBase* col = new IntegerColumn();
    fields_ent.push_back(std::make_pair(col, "a"));
    Entity e("test", fields_ent);

    ih.writeEntity(e);     // Create the entity
    assert(
        ih.validateEntityFieldType("test", "a", "1") &&
        ih.validateEntityFieldType("test", "a", "12345") &&
        !ih.validateEntityFieldType("test", "a", "1.0") &&
        !ih.validateEntityFieldType("test", "a", "string")
    );
    ih.removeEntity("test");
    delete col;
}

/**
 *  Tests that parseEntityAssignField correctly flags invalid assignments
 *  to float fields
 */
void testFieldAssignTypeMismatchFloat() {
    IndexHandler ih;
    Json::Value json;
    defpair fields_ent;

    // Create fields
    ColumnBase* col = new FloatColumn();
    fields_ent.push_back(std::make_pair(col, "a"));
    Entity e("test", fields_ent);     // Create the entity

    ih.writeEntity(e);     // Create the entity
    assert(
        ih.validateEntityFieldType("test", "a", "1.2") &&
        ih.validateEntityFieldType("test", "a", "12.5") &&
        !ih.validateEntityFieldType("test", "a", "string")
    );
    ih.removeEntity("test");
}

/**
 *  Tests that parseEntityAssignField correctly flags invalid assignments
 *   to string fields
 */
void testFieldAssignTypeMismatchString() {
    IndexHandler ih;
    Json::Value json;
    defpair fields_ent;

    // Create fields
    ColumnBase* col = new StringColumn();
    fields_ent.push_back(std::make_pair(col, "a"));
    Entity e("test", fields_ent);
    ih.writeEntity(e);     // Create the entity
    assert(
        ih.validateEntityFieldType("test", "a", "1") &&
        ih.validateEntityFieldType("test", "a", "12345") &&
        ih.validateEntityFieldType("test", "a", "string")
    );
    ih.removeEntity("test");
    delete col;
}

/**
 *  Tests Bayes::countRelations - ensure relation counting is functioning
 *  correctly
 */
void testCountRelations() {
    Bayes bayes;
    IndexHandler ih;
    std::vector<Json::Value> ret;
    defpair fields_ent;
    valpair fields_rel;
    std::unordered_map<std::string, std::string> types;

    // declare three entities
    Entity e1("_w", fields_ent), e2("_x", fields_ent),
        e3("_y", fields_ent), e4("_z", fields_ent);

    // construct a number of relations
    Relation r1("_x", "_y", fields_rel, fields_rel, types, types);
    Relation r2("_x", "_y", fields_rel, fields_rel, types, types);
    Relation r3("_x", "_z", fields_rel, fields_rel, types, types);
    Relation r4("_x", "_z", fields_rel, fields_rel, types, types);
    Relation r5("_w", "_y", fields_rel, fields_rel, types, types);

    ih.removeEntity("_w");
    ih.removeEntity("_x");
    ih.removeEntity("_y");
    ih.removeEntity("_z");

    ih.writeEntity(e1);
    ih.writeEntity(e2);
    ih.writeEntity(e3);
    ih.writeEntity(e4);

    ih.removeRelation(r1);
    ih.removeRelation(r2);
    ih.removeRelation(r3);
    ih.removeRelation(r4);
    ih.removeRelation(r5);

    ih.writeRelation(r1);
    ih.writeRelation(r2);
    ih.writeRelation(r3);
    ih.writeRelation(r4);
    ih.writeRelation(r5);

    AttributeBucket attrs; // empty set of filters

    assert(bayes.countEntityInRelations(e1.name, attrs) == 1);
    assert(bayes.countEntityInRelations(e2.name, attrs) == 4);
    assert(bayes.countEntityInRelations(e3.name, attrs) == 3);
    assert(bayes.countEntityInRelations(e4.name, attrs) == 2);
}

/**
 *  Tests that existsEntityField correctly flags when entity does not
 *   contain a field
 */
void testEntityDoesNotContainField() {
    // TODO - implement
}

/**
 *  Tests Bayes::computeMarginal function - ensure the marginal likelihood
 *  is correct
 */
void testComputeMarginal() {

    Bayes bayes;
    IndexHandler ih;
    std::vector<Json::Value> ret;
    defpair fields_ent;
    valpair fields_rel;
    std::unordered_map<std::string, std::string> types;

    ih.setRelationCountTotal(0);

    // declare three entities
    Entity e1("_w", fields_ent), e2("_x", fields_ent),
        e3("_y", fields_ent), e4("_z", fields_ent);

    // construct a number of relations
    Relation r1("_x", "_y", fields_rel, fields_rel, types, types);
    Relation r2("_x", "_y", fields_rel, fields_rel, types, types);
    Relation r3("_x", "_z", fields_rel, fields_rel, types, types);
    Relation r4("_x", "_z", fields_rel, fields_rel, types, types);
    Relation r5("_w", "_y", fields_rel, fields_rel, types, types);

    ih.removeEntity("_w");
    ih.removeEntity("_x");
    ih.removeEntity("_y");
    ih.removeEntity("_z");

    ih.writeEntity(e1);
    ih.writeEntity(e2);
    ih.writeEntity(e3);
    ih.writeEntity(e4);

    ih.removeRelation(r1);
    ih.removeRelation(r2);
    ih.removeRelation(r3);
    ih.removeRelation(r4);
    ih.removeRelation(r5);

    ih.writeRelation(r1);
    ih.writeRelation(r2);
    ih.writeRelation(r3);
    ih.writeRelation(r4);
    ih.writeRelation(r5);

    ih.setRelationCountTotal(ih.computeRelationsCount("*", "*"));

    // Ensure marginal likelihood reflects the number of relations that
    // contain each entity
    AttributeBucket attrs;

    assert(bayes.computeMarginal(e1.name, attrs) == (float)0.2);
    assert(bayes.computeMarginal(e2.name, attrs) == (float)0.8);
    assert(bayes.computeMarginal(e3.name, attrs) == (float)0.6);
    assert(bayes.computeMarginal(e4.name, attrs) == (float)0.4);

}


/**
 *  Tests that the correct relations are filtered from a set
 */
void testRelation_toJson() {
    valpair left, right;
    std::unordered_map<std::string, std::string> typesL, typesR;

    left.push_back(std::make_pair("x", "1"));
    right.push_back(std::make_pair("y", "2"));
    typesL.insert(std::make_pair("x", COLTYPE_NAME_INT));
    typesR.insert(std::make_pair("y", COLTYPE_NAME_INT));
    Relation rel("x", "y", left, right, typesL, typesR);
    Json::Value json = rel.toJson();
    assert(std::atoi(json[JSON_ATTR_REL_FIELDSL]["x"].asCString()) == 1 &&
        std::atoi(json[JSON_ATTR_REL_FIELDSR]["y"].asCString()) == 2);
}

/**
 *  Ensure sample marginal returns a valid sample
 */
void testSampleMarginal() {
    // Define entities and relations
    // Generate a sample
    // specify filter criteria
    // test sample to ensure that it meets criteria
}

/**
 *  Ensure sample marginal returns a valid sample
 */
void testSamplePairwise() {

    Bayes bayes;
    IndexHandler ih;
    std::vector<Json::Value> ret;
    defpair fields_ent;
    valpair fields_rel;
    std::unordered_map<std::string, std::string> types;

    ih.setRelationCountTotal(0);

    // declare three entities
    Entity e1("_x", fields_ent), e2("_y", fields_ent);

    // construct a number of relations
    Relation r1(e1, e2, fields_rel, fields_rel, types, types);
    Relation r2(e1, e2, fields_rel, fields_rel, types, types);
    Relation r3(e1, e2, fields_rel, fields_rel, types, types);

    if (ih.existsEntity(e1)) ih.removeEntity(e1);
    if (ih.existsEntity(e1)) ih.removeEntity(e2);

    ih.writeEntity(e1);
    ih.writeEntity(e2);

    if (ih.existsRelation(r1)) ih.removeRelation(r1);
    if (ih.existsRelation(r1)) ih.removeRelation(r2);
    if (ih.existsRelation(r1)) ih.removeRelation(r3);

    ih.writeRelation(r1);
    ih.writeRelation(r2);
    ih.writeRelation(r3);

    // Construct Attribute Bucket
    AttributeBucket ab;
    // TODO - assemble a list on which to filter

    // Sample from e1, e2
    Relation r = bayes.samplePairwise(e1, e2, ab);

    // Assert that a sample is generated
    // Assert that the sample matches the attribute set

    // Cleanup
    ih.removeEntity(e1);
    ih.removeEntity(e2);
    ih.removeRelation(r1);
    ih.removeRelation(r2);
    ih.removeRelation(r3);

}

/**
 *  Ensure sample marginal returns a valid sample
 */
void testSamplePairwiseCausal() {
    // Define entities and relations
    // Generate a sample
    // specify filter criteria
    // test sample to ensure that it meets criteria
}

/**
 *  Ensure that the relation filtering is functioning correctly
 */
void testIndexFilterRelationsEQ() {

    AttributeBucket ab;
    AttributeTuple at;
    IndexHandler ih;
    std::vector<Relation> rel_out;

    setRelationsList1();

    // First filter test - One relation meets a single condition
    at = AttributeTuple("_x", "a", "1", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_EQ);
    assert(rel_out.size() == 2);
    assert(rel_out[0].getValue("_x", "a").compare("1") == 0);

    // Second filter test - One relation fails to meet all conditions
    at = AttributeTuple("_x", "a", "0", COLTYPE_NAME_INT);
    ab = AttributeBucket();
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_EQ);
    assert(rel_out.size() == 0);

    // Third filter test - one filtered, one retained
    ab = AttributeBucket();
    at = AttributeTuple("_y", "b", "hello", COLTYPE_NAME_STR);
    ab.addAttribute(at);
    at = AttributeTuple("_x", "a", "1", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_EQ);
    assert(rel_out.size() == 1);
    assert(rel_out[0].getValue("_x", "a").compare("1") == 0);

    releaseObjects();
}


/** Test filtering for > */
void testIndexFilterRelationsGT() {

    AttributeBucket ab;
    AttributeTuple at;
    IndexHandler ih;
    std::vector<Relation> rel_out;

    setRelationsList2();

    // First filter test - All relations greater
    at = AttributeTuple("_x", "a", "0", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GT);
    // TODO - add value asserts here
    assert(rel_out.size() == 2);

    // Second filter test - One relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "5", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GT);
    assert(rel_out[0].getValue("_x", "a").compare("11") == 0);
    assert(rel_out.size() == 1);

    // Third filter test - Neither relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "20", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GT);
    assert(rel_out.size() == 0);

    // Fourth filter test - Float type test
    ab = AttributeBucket();
    at = AttributeTuple("_y", "b", "5.0", COLTYPE_NAME_FLOAT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GT);
    assert(rel_out[0].getValue("_y", "b").compare("12.0") == 0);
    assert(rel_out.size() == 1);

    releaseObjects();
}


/** Test filtering for < */
void testIndexFilterRelationsLT() {
    AttributeBucket ab;
    AttributeTuple at;
    IndexHandler ih;
    std::vector<Relation> rel_out;

    setRelationsList2();

    // First filter test - All relations greater
    at = AttributeTuple("_x", "a", "0", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
    assert(rel_out.size() == 0);

    // Second filter test - One relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "5", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
    assert(rel_out[0].getValue("_x", "a").compare("1") == 0);
    assert(rel_out.size() == 1);

    // Third filter test - Neither relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "20", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
    // TODO - add value asserts here
    assert(rel_out.size() == 2);

    // Fourth filter test - Float type test
    ab = AttributeBucket();
    at = AttributeTuple("_y", "b", "5.0", COLTYPE_NAME_FLOAT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
    assert(rel_out[0].getValue("_y", "b").compare("2.0") == 0);
    assert(rel_out.size() == 1);

    releaseObjects();
}


/** Test filtering for >= */
void testIndexFilterRelationsGTE() {

    AttributeBucket ab;
    AttributeTuple at;
    IndexHandler ih;
    std::vector<Relation> rel_out;

    setRelationsList2();

    // First filter test - Both relations greater or equal
    at = AttributeTuple("_x", "a", "1", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GTE);
    // TODO - add value asserts here
    assert(rel_out.size() == 2);

    // Second filter test - One relation greater than or equal
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "11", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GTE);
    assert(rel_out[0].getValue("_x", "a").compare("11") == 0);
    assert(rel_out.size() == 1);

    releaseObjects();
}


/** Test filtering for <= */
void testIndexFilterRelationsLTE() {

    AttributeBucket ab;
    AttributeTuple at;
    IndexHandler ih;
    std::vector<Relation> rel_out;

    setRelationsList2();

    // First filter test - One relation less than or equal
    at = AttributeTuple("_x", "a", "1", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LTE);
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GTE);
    assert(rel_out[0].getValue("_x", "a").compare("1") == 0);
    assert(rel_out.size() == 1);

    // Second filter test - Both relations less than or equal
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "11", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LTE);
    assert(rel_out.size() == 2);

    releaseObjects();
}

/** Test for counting the presence of entities in relations
        TODO - complete
*/
void testCountEntityInRelations() {
    Bayes b;
    RedisHandler rds(REDISDBTEST, REDISPORT);

    // Create entities and relations
    defpair e1Def, e2Def;
    valpair r1Vals, r2Vals;
    e1Def.push_back(std::make_pair(new IntegerColumn(), "a"));
    e2Def.push_back(std::make_pair(new FloatColumn(), "b"));
    makeTestEntity("_x", e1Def);
    makeTestEntity("_y", e2Def);
    // makeTestRelation("_x", "_y");
    // makeTestRelation("_x", "_y");

    // define relations

    // Persist objects to

    // Define attribute bucket to filter
    AttributeBucket ab;

    // Perform count
    long count = b.countRelations(std::string("_x"), std::string("_y"), ab);

    for (defpair::iterator it = e1Def.begin() ; it != e1Def.end(); ++it)
        delete it->first;
    for (defpair::iterator it = e2Def.begin() ; it != e2Def.end(); ++it)
        delete it->first;

    // Ensure the count is correct
    assert(count == 1);
}


/** Entity / Relation ORM tests **/

/** Test entity writing */
void testEntityWrite() {
    IndexHandler ih;
    std::string name = "_x";
    std::string field_name = "a";
    defpair e1Def;

    // setup test entity
    e1Def.push_back(std::make_pair(new IntegerColumn(), field_name));
    makeTestEntity(name, e1Def);
    writeEntities();

    // Fetch the entity written
    Json::Value value;
    ih.fetchEntity(name, value);

    // Ensure name matches and attribute exists
    assert(std::strcmp(value[JSON_ATTR_ENT_ENT].asCString(),
        name.c_str()) == 0);
    assert(value[JSON_ATTR_ENT_FIELDS].isMember(field_name));
    assert(std::strcmp(value[JSON_ATTR_ENT_FIELDS][field_name].asCString(),
        COLTYPE_NAME_INT) == 0);

    // Cleanup
    removeEntities();
    releaseObjects();

    for (defpair::iterator it = e1Def.begin() ; it != e1Def.end(); ++it)
        delete it->first;

    // test removal
    Json::Value json;
    ih.fetchRaw("*", json);
    assert(json.empty());
}


/** Test relation writing */
void testRelationWrite() {
    std::string left_name = "_x";
    std::string right_name = "_y";

    std::string field_left_name = "a";
    std::string field_right_name = "b";

    std::string field_left_val = "1";
    std::string field_right_val = "2";

    std::unordered_map<std::string, std::string> left_types, right_types;
    defpair e1Def, e2Def;
    valpair e1Val, e2Val;

    // setup test entity
    e1Def.push_back(std::make_pair(new IntegerColumn(), field_left_name));
    e2Def.push_back(std::make_pair(new IntegerColumn(), field_right_name));
    makeTestEntity(left_name, e1Def);
    makeTestEntity(right_name, e2Def);
    writeEntities();

    e1Val.push_back(std::make_pair(field_left_name, field_left_val));
    e2Val.push_back(std::make_pair(field_right_name, field_right_val));
    left_types.insert(std::make_pair("a", COLTYPE_NAME_INT));
    right_types.insert(std::make_pair("b", COLTYPE_NAME_INT));
    makeTestRelation(left_name, right_name, e1Val, e2Val, left_types,
        right_types);
    writeRelations();

    // Fetch the relation written
    IndexHandler ih;
    Json::Value json;
    std::string key;
    for (std::vector<Relation>::iterator it = openRelations.begin();
            it != openRelations.end(); ++it) {
        ih.fetchRaw(it->generateKey(), json);
        assert(std::strcmp(json[JSON_ATTR_REL_ENTL].asCString(),
            it->name_left.c_str()) == 0);
        assert(std::strcmp(json[JSON_ATTR_REL_ENTR].asCString(),
            it->name_right.c_str()) == 0);
        assert(json[JSON_ATTR_REL_FIELDSL].isMember(field_left_name));
        assert(json[JSON_ATTR_REL_FIELDSR].isMember(field_right_name));
        assert(json[JSON_ATTR_REL_FIELDSL][JSON_ATTR_FIELDS_COUNT].asInt()
            == 1);
        assert(json[JSON_ATTR_REL_FIELDSR][JSON_ATTR_FIELDS_COUNT].asInt()
            == 1);
        assert(std::strcmp(json[JSON_ATTR_REL_FIELDSL][
            std::string(JSON_ATTR_REL_TYPE_PREFIX) + field_left_name].
            asCString(), COLTYPE_NAME_INT) == 0);
        assert(std::strcmp(json[JSON_ATTR_REL_FIELDSR][
            std::string(JSON_ATTR_REL_TYPE_PREFIX) + field_right_name].
            asCString(), COLTYPE_NAME_INT) == 0);
    }

    for (defpair::iterator it = e1Def.begin() ; it != e1Def.end(); ++it)
        delete it->first;
    for (defpair::iterator it = e2Def.begin() ; it != e2Def.end(); ++it)
        delete it->first;

    // Cleanup
    removeEntities();
    removeRelations();
    releaseObjects();

    // Test Removal
    ih.fetchRaw("*", json);
    // TODO - determine why this relation isn't removed
    // assert(json.empty());

}


/**
 *  Tests that removal of relations cascading on entities functions properly
 */
void testEntityCascadeRemoval() {
    // TODO - implement
}

/**
 *  Tests ...
 */
void testADDREL() {
    // TODO - implement
}

/**
 *  Tests ...
 */
void testDEF() {
    // TODO - implement
}

/**
 *  Initialize tests that should (and should not) run
 */
void initTests() {

    // Redis Tests
    tests.insert(std::make_pair("testRedisSetGetRemove",
        std::make_pair(true, testRedisSetGetRemove)));
    tests.insert(std::make_pair("testRedisKeys",
        std::make_pair(true, testRedisKeys)));

    tests.insert(std::make_pair("testMd5Hashing",
        std::make_pair(true, testMd5Hashing)));
    tests.insert(std::make_pair("testRegexForTypes",
        std::make_pair(true, testRegexForTypes)));
    tests.insert(std::make_pair("testOrderPairAlphaNumeric",
        std::make_pair(true, testOrderPairAlphaNumeric)));

    tests.insert(std::make_pair("testJSONEntityEncoding",
        std::make_pair(false, testJSONEntityEncoding)));
    tests.insert(std::make_pair("testJSONRelationEncoding",
        std::make_pair(false, testJSONRelationEncoding)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchInteger",
        std::make_pair(false, testFieldAssignTypeMismatchInteger)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchFloat",
        std::make_pair(false, testFieldAssignTypeMismatchFloat)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchString",
        std::make_pair(false, testFieldAssignTypeMismatchString)));
    tests.insert(std::make_pair("testCountRelations",
        std::make_pair(false, testCountRelations)));

    // Tests for filtering
    tests.insert(std::make_pair("testIndexFilterRelationsEQ",
        std::make_pair(true, testIndexFilterRelationsEQ)));
    tests.insert(std::make_pair("testIndexFilterRelationsGT",
        std::make_pair(true, testIndexFilterRelationsGT)));
    tests.insert(std::make_pair("testIndexFilterRelationsLT",
        std::make_pair(true, testIndexFilterRelationsLT)));
    tests.insert(std::make_pair("testIndexFilterRelationsGTE",
        std::make_pair(true, testIndexFilterRelationsGTE)));
    tests.insert(std::make_pair("testIndexFilterRelationsLTE",
        std::make_pair(true, testIndexFilterRelationsLTE)));
    tests.insert(std::make_pair("testRelation_toJson",


        std::make_pair(true, testRelation_toJson)));
    tests.insert(std::make_pair("testCountEntityInRelations",
        std::make_pair(false, testCountEntityInRelations)));

    // Tests for test writing and ORM methods
    tests.insert(std::make_pair("testEntityWrite",
        std::make_pair(true, testEntityWrite)));
    tests.insert(std::make_pair("testRelationWrite",
        std::make_pair(true, testRelationWrite)));
    tests.insert(std::make_pair("testEntityCascadeRemoval",
        std::make_pair(false, testEntityCascadeRemoval)));

    // Test CLI Commands
    tests.insert(std::make_pair("testADDREL",
        std::make_pair(false, testADDREL)));
    tests.insert(std::make_pair("testGEN",
        std::make_pair(false, testADDREL)));
}

/** MAIN BLOCK -- Execute tests */
int main() {

    initTests();
    cout << endl << "-- TESTS BEGIN --" << endl << endl;

    for (std::unordered_map<std::string,
            std::pair<bool, FnPtr>>::iterator it = tests.begin();
            it != tests.end(); ++it) {
        if (it->second.first) {
            cout << "----- TESTING: " << it->first << endl << endl;
            it->second.second();
            cout << endl << "TEST PASSED." << endl << endl;
        }
    }

    cout << endl << "-- TESTS END --" << endl;

    return 0;
}
