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
#include <vector>

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
    std::vector<std::string>* keys(std::string);
};

/** Establishes a connection to a redis instance */
void RedisHandler::connect() { this->context = redisConnect(REDISHOST, REDISPORT); }

/** Writes a key value to redis */
void RedisHandler::write(std::string key, std::string value) {
    redisCommand(this->context, "SET %s %s", key.c_str(), value.c_str());
}

/** Read a value from redis given a key */
std::string RedisHandler::read(std::string key) {
    std::string result;
    redisReply *reply = (redisReply*)redisCommand(this->context, "GET %s", key.c_str());
    result = reply->str;
    freeReplyObject(reply);
    return result;
}

/** Read a value from redis given a key pattern */
std::vector<std::string>* RedisHandler::keys(std::string pattern) {
    std::vector<string>* elems = new std::vector<string>();
    redisReply *reply = (redisReply*)redisCommand(this->context, "KEYS %s", pattern.c_str());

    // Determine if the reply is an array and iterate through elems if so
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int j = 0; j < reply->elements; j++) {
            elems->push_back(reply->element[j]->str);
        }
    }

    freeReplyObject(reply);
    return elems;
}

#endif
