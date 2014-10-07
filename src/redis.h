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
#include "hiredis/hiredis.h"

#define REDISHOST "127.0.0.1"
#define REDISPORT 6379


using namespace std;


/**
 *  Defines interface to redis server
 */
class RedisHandler {

    std::string host;
    std::string port;

    redisContext *context;
    
public:
    RedisHandler();
    RedisHandler(std::string, std::string);
    
    bool connect();
    bool write(std::string, std::string);
    std::string read(std::string);
    std::string keys(std::string);
};


/**
 *  Constructor
 */
RedisHandler::RedisHandler() { this->host = REDISHOST; this->port = REDISPORT; }


/**
 *  Constructor that allows specification of host and db
 */
RedisHandler::RedisHandler(std::string host, std::string port) { this->host = host; this->port = port; }


/**
 *  Establishes a connection to a redis instance
 */
void RedisHandler::connect() {
    this->context = redisConnect(REDISHOST, REDISPORT);
    return true;
}


/** Writes a key value to redis */
void RedisHandler::write(std::string key, std::string value) { redisCommand(c, "SET %s %s", key, value); }


///**
// *  Read a value from redis given a key
// */
//std::string RedisHandler::read(std::string key) {
//    redis3m::reply r = this->conn->run(redis3m::command("GET") << key );
//    return r.str();
//}
//
///**
// *  Read a value from redis given a key
// */
//std::string RedisHandler::keys(std::string pattern) {
//    redis3m::reply r = this->conn->run(redis3m::command("KEYS") << pattern );
//    return r.str();
//}

#endif
