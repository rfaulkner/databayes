/*
 *  redis.h
 *
 *  Handles the interface to redis
 *
 *  Created by Ryan Faulkner on 2014-06-12
 *  Copyright (c) 2014. All rights reserved.
 */

#ifndef _redis_h
#define _redis_h

#include <iostream>
#include <string>
#include <redis3m/redis3m.hpp>

using namespace std;


/**
 *  Defines interface to redis server
 */
class RedisHandler {
    string host;
    string database;
    redis3m::connection::ptr_t conn;
    
public:
    RedisHandler() {}
    RedisHandler(string, string);
    
    bool connect();
    bool write(string, string);
    string read(string);
};


/**
 *  Constructor that allows specification of host and db
 */
RedisHandler::RedisHandler(string host, string database) {
    this->host = host;
    this->database = database;
}


/**
 *  Establishes a connection to a redis instance
 */
bool RedisHandler::connect() {
    redis3m::connection::ptr_t conn = redis3m::connection::create();
    return true;
}


/**
 *  Writes a key value to redis
 */
bool RedisHandler::write(string key, string value) {
    redis3m::conn->run(redis3m::command("SET") << key << value );
    return true;
}


/**
 *  Read a value from redis given a key
 */
string RedisHandler::read(string key) {
    redis3m::reply r = conn->run(command("GET") << key );
    return reply;
}

#endif
