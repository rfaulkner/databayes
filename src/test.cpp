
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

#include "redis.h"

#define REDISHOST "localhost"
#define REDISDB "databayes_test"

using namespace std;

/**
 * Test to ensure that redis keys are correctly returned
 */
bool testRedisKeys() {
    RedisHandler* redisHandler = new RedisHandler(REDISHOST, REDISDB);
    redisHandler->write("k1", "v1");
    redisHandler->write("k2", "v2");
    cout << redisHandler->keys("*") << endl;
}

int main() {
    testRedisKeys();
    return 0;
}
