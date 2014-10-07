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
    int port;

    redisContext *context;
    
public:
    RedisHandler() { this->host = REDISHOST; this->port = REDISPORT; }
    RedisHandler(std::string, int) { this->host = host; this->port = port; }
    
    void connect();
    void write(std::string, std::string);
    std::string read(std::string);
    std::string keys(std::string);
};


/** Establishes a connection to a redis instance */
void RedisHandler::connect() { this->context = redisConnect(REDISHOST, REDISPORT); }


/** Writes a key value to redis */
void RedisHandler::write(std::string key, std::string value) {
    redisCommand(this->context, "SET %s %s", key.c_str(), value.c_str());
}


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
