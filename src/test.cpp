
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

#include "column_types.h"
#include "redis.h"
#include "md5.h"
#include "index.h"
#include "bayes.h"
#include "model.h"

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
    for (std::vector<Entity>::iterator it = openEntities.begin(); it != openEntities.end(); ++it)
        ih.removeEntity(*it);
    for (std::vector<Relation>::iterator it = openRelations.begin(); it != openRelations.end(); ++it)
        ih.removeRelation(*it);
    openEntities.clear();
    openRelations.clear();
}


/* -- SETUP FUNCTIONS -- */

/* Builds a vector of relations */
void setRelationsList1() {

    // Initialize fields and types
    std::vector<std::pair<ColumnBase, std::string>> fields_ent_1, fields_ent_2;
    std::vector<std::pair<std::string, std::string>> fields_rel_1, fields_rel_2, fields_rel_3;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    fields_ent_1.push_back(std::make_pair(IntegerColumn(), "a"));
    fields_ent_2.push_back(std::make_pair(StringColumn(), "b"));

    fields_rel_1.push_back(std::make_pair("a", "1"));
    fields_rel_2.push_back(std::make_pair("b", "hello"));
    fields_rel_3.push_back(std::make_pair("b", "goodbye"));

    types_rel_1.insert(std::make_pair("a", COLTYPE_NAME_INT));
    types_rel_2.insert(std::make_pair("b", COLTYPE_NAME_STR));

    // Initialize entities
    makeTestEntity("_x", fields_ent_1);
    makeTestEntity("_y", fields_ent_2);

    // Add relation set
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_2, types_rel_1, types_rel_2);
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_3, types_rel_1, types_rel_2);
}

/* Builds a vector of relations */
void setRelationsList2() {

    // Initialize fields and types
    std::vector<std::pair<ColumnBase, std::string>> fields_ent_1, fields_ent_2;
    std::vector<std::pair<std::string, std::string>> fields_rel_1, fields_rel_2, fields_rel_3, fields_rel_4;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    fields_ent_1.push_back(std::make_pair(IntegerColumn(), "a"));
    fields_ent_2.push_back(std::make_pair(FloatColumn(), "b"));

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
    makeTestRelation("_x", "_y", fields_rel_1, fields_rel_2, types_rel_1, types_rel_2);
    makeTestRelation("_x", "_y", fields_rel_3, fields_rel_4, types_rel_1, types_rel_2);
}

/* -- TEST FUNCTIONS -- */

/** Test to ensure that redis keys are correctly returned */
void testRedisSet() {
    RedisHandler r;
    r.connect();
    r.write("foo", "bar");
}

/** Test to ensure that redis keys are correctly returned */
void testRedisGet() {
    RedisHandler r;
    r.connect();
    cout << endl << "VALUE FOR KEY foo" << endl;
    cout << r.read("foo") << endl << endl;
}

/** Test to ensure that redis keys are correctly returned */
void testRedisKeys() {
    RedisHandler r;
    std::vector<std::string> vec;

    r.connect();
    vec = r.keys("*");
    cout << "KEY LIST FOR *" << endl;
    for (std::vector<std::string>::iterator it = vec.begin() ; it != vec.end(); ++it) {
        cout << *it << endl;
    }
    cout << endl;
}

/** Test to ensure that redis keys are correctly returned */
void testRedisIO() {
    RedisHandler r;
    std::vector<string>* vec;
    std::string outTrue, outFalse = "";

    r.connect();
    r.write("test_key", "test value");
    outTrue = r.read("test_key");
    assert(std::strcmp(outTrue.c_str(), "test value") == 0);
    r.deleteKey("test_key");
    assert(!r.exists("test_key"));
}

/** Test to ensure that md5 hashing works */
void testMd5Hashing() {
    cout << endl << "md5 of 'mykey': " << md5("mykey") << endl;
}

/** Test to ensure that md5 hashing works */
void testRegexForTypes() {
    IntegerColumn ic;
    FloatColumn fc;
    assert(ic.validate("1981"));
    assert(fc.validate("5.2"));
    cout << "Passed regex tests." << endl;
}


/** Test to ensure that md5 hashing works */
void testOrderPairAlphaNumeric() {
    IndexHandler ih;
    assert(std::strcmp(ih.orderPairAlphaNumeric("b", "a").c_str(), "a_b") == 0);
    cout << "Passed orderPairAlphaNumeric tests." << endl;
}

/**
 * Test to ensure that relation entities are encoded properly
 */
void testJSONEntityEncoding() {

    IndexHandler ih;
    Json::Value json;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;

    fields_ent.push_back(std::make_pair(IntegerColumn(), "a"));   // Create fields
    Entity e("test", fields_ent);
    ih.writeEntity(e);     // Create the entity
    ih.fetchEntity("test", json);           // Fetch the entity representation
    cout << "TEST ENTITY:" << endl << endl << json.toStyledString() << endl;

    // Assert that entity as read matches definition
    assert(std::strcmp(json["entity"].asCString(), "test") == 0 &&
            std::strcmp(json["fields"]["a"].asCString(), "integer") == 0 &&
            json["fields"]["_itemcount"].asInt() == 1
    );
    ih.removeEntity("test");                // Remove the entity
}


/**
 * Test to ensure that relation fields are encoded properly
 */
void testJSONRelationEncoding() {
    IndexHandler ih;
    std::vector<Json::Value> ret;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent_1;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent_2;
    std::vector<std::pair<std::string, std::string>> fields_rel_1;
    std::vector<std::pair<std::string, std::string>> fields_rel_2;
    std::unordered_map<std::string, std::string> types_rel_1, types_rel_2;

    // Popualate fields
    fields_ent_1.push_back(std::make_pair(IntegerColumn(), "a"));
    fields_ent_2.push_back(std::make_pair(StringColumn(), "b"));
    fields_rel_1.push_back(std::make_pair("a", "1"));
    fields_rel_2.push_back(std::make_pair("b", "hello"));
    types_rel_1.insert(std::make_pair("a", COLTYPE_NAME_INT));
    types_rel_2.insert(std::make_pair("b", COLTYPE_NAME_FLOAT));

    // Create entities
    Entity e1("test_1", fields_ent_1), e2("test_2", fields_ent_2);
    ih.writeEntity(e1);
    ih.writeEntity(e2);

    // Create relation in redis
    Relation r("test_1", "test_2", fields_rel_1, fields_rel_2, types_rel_1, types_rel_2);
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
}


/**
 *  Tests that parseEntityAssignField correctly flags invalid assignments to integer fields
 */
void testFieldAssignTypeMismatchInteger() {
    IndexHandler ih;
    Json::Value json;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    fields_ent.push_back(std::make_pair(IntegerColumn(), "a"));   // Create fields
    Entity e("test", fields_ent);

    ih.writeEntity(e);     // Create the entity
    assert(
        ih.validateEntityFieldType("test", "a", "1") &&
        ih.validateEntityFieldType("test", "a", "12345") &&
        !ih.validateEntityFieldType("test", "a", "1.0") &&
        !ih.validateEntityFieldType("test", "a", "string")
    );
    ih.removeEntity("test");
}

/**
 *  Tests that parseEntityAssignField correctly flags invalid assignments to float fields
 */
void testFieldAssignTypeMismatchFloat() {
    IndexHandler ih;
    Json::Value json;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    fields_ent.push_back(std::make_pair(FloatColumn(), "a"));   // Create fields
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
 *  Tests that parseEntityAssignField correctly flags invalid assignments to string fields
 */
void testFieldAssignTypeMismatchString() {
    IndexHandler ih;
    Json::Value json;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    fields_ent.push_back(std::make_pair(StringColumn(), "a"));   // Create fields
    Entity e("test", fields_ent);
    ih.writeEntity(e);     // Create the entity
    assert(
        ih.validateEntityFieldType("test", "a", "1") &&
        ih.validateEntityFieldType("test", "a", "12345") &&
        ih.validateEntityFieldType("test", "a", "string")
    );
    ih.removeEntity("test");
}

/**
 *  Tests Bayes::countRelations - ensure relation counting is functioning correctly
 */
void testCountRelations() {
    Bayes bayes;
    IndexHandler ih;
    std::vector<Json::Value> ret;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    std::vector<std::pair<std::string, std::string>> fields_rel;
    std::unordered_map<std::string, std::string> types;

    // declare three entities
    Entity e1("_w", fields_ent), e2("_x", fields_ent), e3("_y", fields_ent), e4("_z", fields_ent);

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
 *  Tests that existsEntityField correctly flags when entity does not contain a field
 */
void testEntityDoesNotContainField() {
    // TODO - implement
}

/**
 *  Tests Bayes::computeMarginal function - ensure the marginal likelihood is correct
 */
void testComputeMarginal() {

    Bayes bayes;
    IndexHandler ih;
    std::vector<Json::Value> ret;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    std::vector<std::pair<std::string, std::string>> fields_rel;
    std::unordered_map<std::string, std::string> types;

    ih.setRelationCountTotal(0);

    // declare three entities
    Entity e1("_w", fields_ent), e2("_x", fields_ent), e3("_y", fields_ent), e4("_z", fields_ent);

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

    // Ensure marginal likelihood reflects the number of relations that contain each entity
    AttributeBucket attrs;

    assert(bayes.computeMarginal(e1.name, attrs) == (float)0.2);
    assert(bayes.computeMarginal(e2.name, attrs) == (float)0.8);
    assert(bayes.computeMarginal(e3.name, attrs) == (float)0.6);
    assert(bayes.computeMarginal(e4.name, attrs) == (float)0.4);

}

/**
 *  Tests that removal of entities functions properly
 */
void testEntityRemoval() {

    IndexHandler ih;
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;

    // declare three entities
    Entity e("_w", fields_ent);

    if (ih.existsEntity(e))
        ih.removeEntity(e);
    ih.writeEntity(e);
    ih.removeEntity(e);

    assert(ih.existsEntity(e));
}

/**
 *  Tests that removal of relations functions properly
 */
void testRelationRemoval() {
    // TODO - implement
}

/**
 *  Tests that removal of relations cascading on entities functions properly
 */
void testEntityCascadeRemoval() {
    // TODO - implement
}

/**
 *  Tests that the correct relations are filtered from a set
 */
void testRelationFiltering() {
    // TODO - implement
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
    assert(std::atoi(json[JSON_ATTR_REL_FIELDSL]["x"].asCString()) == 1 && std::atoi(json[JSON_ATTR_REL_FIELDSR]["y"].asCString()) == 2);
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
    std::vector<std::pair<ColumnBase, std::string>> fields_ent;
    std::vector<std::pair<std::string, std::string>> fields_rel;
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
    assert(rel_out.size() == 2);

    // Second filter test - One relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "5", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GT);
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
    assert(rel_out.size() == 1);

    // Third filter test - Neither relation greater
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "20", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
    assert(rel_out.size() == 2);

    // Fourth filter test - Float type test
    ab = AttributeBucket();
    at = AttributeTuple("_y", "b", "5.0", COLTYPE_NAME_FLOAT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LT);
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
    assert(rel_out.size() == 2);

    // Second filter test - One relation greater than or equal
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "10", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_GTE);
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
    assert(rel_out.size() == 1);

    // Second filter test - Both relations less than or equal
    ab = AttributeBucket();
    at = AttributeTuple("_x", "a", "10", COLTYPE_NAME_INT);
    ab.addAttribute(at);
    rel_out = openRelations;
    ih.filterRelations(rel_out, ab, ATTR_TUPLE_COMPARE_LTE);
    assert(rel_out.size() == 2);

    releaseObjects();
}


/**
 *  Initialize tests that should (and should not) run
 */
void initTests() {

    tests.insert(std::make_pair("testRedisSet", std::make_pair(false, testRedisSet)));
    tests.insert(std::make_pair("testRedisGet", std::make_pair(false, testRedisGet)));
    tests.insert(std::make_pair("testRedisKeys", std::make_pair(false, testRedisKeys)));
    tests.insert(std::make_pair("testMd5Hashing", std::make_pair(false, testMd5Hashing)));
    tests.insert(std::make_pair("testRedisIO", std::make_pair(false, testRedisIO)));
    tests.insert(std::make_pair("testRegexForTypes", std::make_pair(false, testRegexForTypes)));
    tests.insert(std::make_pair("testOrderPairAlphaNumeric", std::make_pair(false, testOrderPairAlphaNumeric)));
    tests.insert(std::make_pair("testJSONEntityEncoding", std::make_pair(false, testJSONEntityEncoding)));
    tests.insert(std::make_pair("testJSONRelationEncoding", std::make_pair(false, testJSONRelationEncoding)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchInteger", std::make_pair(false, testFieldAssignTypeMismatchInteger)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchFloat", std::make_pair(false, testFieldAssignTypeMismatchFloat)));
    tests.insert(std::make_pair("testFieldAssignTypeMismatchString", std::make_pair(false, testFieldAssignTypeMismatchString)));
    tests.insert(std::make_pair("testCountRelations", std::make_pair(false, testCountRelations)));
    tests.insert(std::make_pair("testIndexFilterRelationsEQ", std::make_pair(true, testIndexFilterRelationsEQ)));
    tests.insert(std::make_pair("testIndexFilterRelationsGT", std::make_pair(true, testIndexFilterRelationsGT)));
    tests.insert(std::make_pair("testIndexFilterRelationsLT", std::make_pair(false, testIndexFilterRelationsLT)));
    tests.insert(std::make_pair("testIndexFilterRelationsGTE", std::make_pair(false, testIndexFilterRelationsGTE)));
    tests.insert(std::make_pair("testIndexFilterRelationsLTE", std::make_pair(false, testIndexFilterRelationsLTE)));
    tests.insert(std::make_pair("testRelation_toJson", std::make_pair(false, testRelation_toJson)));
}

/** MAIN BLOCK -- Execute tests */
int main() {

    initTests();
    cout << endl << "-- TESTS BEGIN --" << endl << endl;

    for (std::unordered_map<std::string, std::pair<bool, FnPtr>>::iterator it = tests.begin(); it != tests.end(); ++it) {
        if (it->second.first) {
            cout << "----- TESTING: " << it->first << endl << endl;
            it->second.second();
            cout << endl << "TEST PASSED." << endl << endl;
        }
    }

    cout << endl << "-- TESTS END --" << endl;

    return 0;
}
