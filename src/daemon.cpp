

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
#include <string>
#include <redis3m/connection.h>
#include "parse.h"
#include "redis.h"

// Daemon Macros
#define DBY_CMD_QUEUE_LOCK_SUFFIX "_lock"
#define DBY_CMD_QUEUE_PREFIX "dby_command_queue_"
#define DBY_RSP_QUEUE_PREFIX "dby_response_queue_"

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
long getKeyOrderValue(Parser& p, std::string key) {
    vector<std::string> pieces = p.tokenize(key);
    if (pieces.length() > 0)
        return pieces[pieces.length() - 1];
    else
        return -1;  // error
}

int main() {
    std::string line;
    Parser* parser = new Parser();
    RedisHandler* redisHandler = new RedisHandler(REDISHOST, REDISPORT);
    parser->setDaemon(true);

    cout << "" << endl;

    // Read the input
    while (1) {

        // 1. Fetch next command off the queue
        this->redisHandler->connect();
        std::string key = getNextQueueKey(*redisHandler);
        std::string lock;
        if (this->redisHandler->exists(key) && strcmp(key.c_str(), "") != 0) {
            line = redisHandler->read(key);

            // 2. LOCK KEY.  If it's already locked try again
            lock = std::string(DBY_CMD_QUEUE_LOCK_SUFFIX) + key;
            if (!redisHandler->exists(lock)) {
                redisHandler->write(lock, "1"); // lock it
            } else {
                std::chrono::milliseconds(1);
                continue;
            }
        }

        // 3. Parse the command and write response to redis
        long key_value = getKeyOrderValue(key);
        redisHandler->write(std::string(DBY_RSP_QUEUE_PREFIX) + key_value, parser->parse(line));
        parser->resetState();

        // 4. Remove the element and it's lock
        redisHandler->deleteKey(key);
        redisHandler->deleteKey(lock);

        // 5. execute timeout
        std::chrono::milliseconds(10);
    }
    return 0;
}


