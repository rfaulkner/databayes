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
    std::string host;
    std::string database;
    redis3m::connection::ptr_t conn;
    
public:
    RedisHandler() {}
    RedisHandler(std::string, std::string);
    
    bool connect();
    bool write(std::string, std::string);
    string read(std::string);
};


/**
 *  Constructor that allows specification of host and db
 */
RedisHandler::RedisHandler(std::string host, std::string database) {
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
bool RedisHandler::write(std::string key, std::string value) {
    this->conn->run(redis3m::command("SET") << key << value );
    return true;
}


/**
 *  Read a value from redis given a key
 */
string RedisHandler::read(std::string key) {
    redis3m::reply r = this->conn->run(redis3m::command("GET") << key );
    return r.str();
}

#endif
