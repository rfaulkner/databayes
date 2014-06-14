
/*
 *  redis.cpp
 *
 *  Handles the interface to redis
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#include <iostream>
#include <string>

using namespace std;

class RedisHandler {
    string host;
    string database;
public:
    RedisHandler() {}
    RedisHandler(string, string);
    bool connect();
    bool write(string key, int value);
    int read(string key);
};

RedisHandler::RedisHandler(string host, string database) {
    this->host = host;
    this->database = database;
}
