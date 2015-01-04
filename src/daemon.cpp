

/*
 *  client.cpp
 *
 *  Handles the client env.
 *
 *  Created by Ryan Faulkner on 2014-06-08
 *  Copyright (c) 2014. All rights reserved.
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include <redis3m/connection.h>
#include "parse.h"
#include "redis.h"

// Daemon Macros
#define DBY_CMD_QUEUE_LOCK_SUFFIX "_lock"
#define DBY_CMD_QUEUE_PREFIX "dby_command_queue_"
#define DBY_RSP_QUEUE_PREFIX "dby_response_queue_"

#define REDIS_POLL_TIMEOUT 2000
#define REDIS_RETRY_TIMEOUT 1000


using namespace std;


/**
 * This method handles fetching the next item to be
 *
 * TODO - handle ordering
 */
std::string getNextQueueKey(RedisHandler& rh) {
    std::vector<std::string> keys = rh.keys(std::string(DBY_CMD_QUEUE_PREFIX) + std::string("*"));
    if (keys.size() > 0)
        return keys[0];
    else
        return "";
}

/**
 * This method handles extracting the value from the redis command key.  The parser
 * functionality to tokenize strings is used to assist
 */
std::string getKeyOrderValue(Parser& p, std::string key) {
    vector<std::string> pieces = p.tokenize(key, '_');
    std::string number;
    std::stringstream strstream;

    if (pieces.size() > 0) {
        strstream << pieces[pieces.size() - 1];
        strstream >> number;
        return number;
    } else
        return "";  // error
}

int main() {
    std::string line;
    std::string lock;
    std::string key;

    Parser* parser = new Parser();
    RedisHandler* redisHandler = new RedisHandler(REDISHOST, REDISPORT);

    cout << "Running databayes daemon..." << endl;

    // Read the input
    while (1) {

        // 1. execute timeout
        std::this_thread::sleep_for(std::chrono::milliseconds(REDIS_POLL_TIMEOUT));

        // 2. Fetch next command off the queue
        redisHandler->connect();
        key = getNextQueueKey(*redisHandler);
        lock = "";

        if (redisHandler->exists(key) && strcmp(key.c_str(), "") != 0) {
            line = redisHandler->read(key);

            // 3. LOCK KEY.  If it's already locked try again
            lock = std::string(DBY_CMD_QUEUE_LOCK_SUFFIX) + key;
            if (!redisHandler->exists(lock)) {
                redisHandler->write(lock, "1"); // lock it
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(REDIS_RETRY_TIMEOUT));
                continue;   // locked, try again after a timeout
            }
        } else {
            continue;  // If the key does not exist the
        }

        // 4. Parse the command and write response to redis
        std::string key_value = getKeyOrderValue(*parser, key);

        // 5. Parse the command and write response to redis
        if (std::strcmp(key_value.c_str(), "") != 0) {
            redisHandler->write(std::string(DBY_RSP_QUEUE_PREFIX) + key_value, parser->parse(line));
            parser->resetState();
        } else {
            // Badly formed key, drop the key and remove the lock
            cout << key + std::string(" is badly formed, can't determine value - not processed.") << endl;
        }

        // 6. Remove the element and it's lock
        redisHandler->deleteKey(key);
        redisHandler->deleteKey(lock);
    }
    return 0;
}


