
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
#include <assert.h>

#include "redis.h"
#include "md5.h"

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
    std::vector<string>* vec;

    r.connect();
    vec = r.keys("*");
    cout << "KEY LIST FOR *" << endl;
    for (std::vector<std::string>::iterator it = vec->begin() ; it != vec->end(); ++it) {
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
void md5Hashing() {
    cout << endl << "md5 of 'mykey': " << md5("mykey") << endl;
}

int main() {

//    testRedisSet();
//    testRedisGet();
//    testRedisKeys();
//    md5Hashing();
    testRedisIO();

    cout << endl;

    return 0;
}
