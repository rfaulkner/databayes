
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

#define REDISHOST "127.0.0.1"
#define REDISPORT 6379

using namespace std;

/** Test to ensure that redis keys are correctly returned */
void testRedisSet() {
    RedisHandler r;
    r.connect();
    r.write("foo", "bar");
}

int main() {
    testRedisSet();
    return 0;
}
