
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

#define REDISHOST "127.0.0.1"
#define REDISPORT 6379

using namespace std;

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
    std::vector<std::pair<ColumnBase*, std::string>>* fields_ent = new vector<std::pair<ColumnBase*, std::string>>;

    fields_ent->push_back(std::make_pair(getColumnType("integer"), "a"));   // Create fields
    ih.writeEntity("test", fields_ent);     // Create the entity
    ih.fetchEntity("test", json);           // Fetch the entity representation
    cout << "TEST ENTITY:" << endl << endl << json.toStyledString() << endl;

    // Assert that entity as read matches definition
    assert(std::strcmp(json["entity"].asCString(), "test") == 0 &&
            std::strcmp(json["fields"]["a"].asCString(), "integer") == 0 &&
            json["fields"]["fields_count"].asInt() == 1
    );
    ih.removeEntity("test");                // Remove the entity
}


/**
 * Test to ensure that relation fields are encoded properly
 */
void testJSONRelationEncoding() {
    IndexHandler ih;
    std::vector<Json::Value> ret;
    std::vector<std::pair<ColumnBase*, std::string>>* fields_ent_1 = new vector<std::pair<ColumnBase*, std::string>>;
    std::vector<std::pair<ColumnBase*, std::string>>* fields_ent_2 = new vector<std::pair<ColumnBase*, std::string>>;
    std::vector<std::pair<std::string, std::string>>* fields_rel_1 = new vector<std::pair<std::string, std::string>>;
    std::vector<std::pair<std::string, std::string>>* fields_rel_2 = new vector<std::pair<std::string, std::string>>;

    // Popualate fields
    fields_ent_1->push_back(std::make_pair(getColumnType("integer"), "a"));
    fields_ent_2->push_back(std::make_pair(getColumnType("string"), "b"));
    fields_rel_1->push_back(std::make_pair("a", "1"));
    fields_rel_2->push_back(std::make_pair("b", "hello"));

    // Create entities
    ih.writeEntity("test_1", fields_ent_1);
    ih.writeEntity("test_2", fields_ent_1);

    // Create relation in redis
    ih.writeRelation("test_1", "test_2", fields_rel_1, fields_rel_2);

    // Fetch the entity representation
    ret = ih.fetchRelationPrefix("test_1", "test_2");
    for (std::vector<Json::Value>::iterator it = ret.begin(); it != ret.end(); ++it)
        cout << (*it).toStyledString() << endl;

    // assert
}

int main() {
    cout << "-- TESTS BEGIN --" << endl << endl;

//    testRedisSet();
//    testRedisGet();
//    testRedisKeys();
//    md5Hashing();
//    testRedisIO();
//    testRegexForTypes();
//    testOrderPairAlphaNumeric();
    testJSONEntityEncoding();

    cout << endl << "-- TESTS END --" << endl;

    return 0;
}
