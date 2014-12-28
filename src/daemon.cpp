

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

#define DBY_CMD_QUEUE_LOCK_SUFFIX "_lock"
#define DBY_CMD_QUEUE_PREFIX "dby_command_queue_"

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

int main() {
    std::string line;
    Parser* parser = new Parser();
    RedisHandler* redisHandler = new RedisHandler(REDISHOST, REDISPORT);
    parser->setDebug(true);

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
            lock = key + std::string(DBY_CMD_QUEUE_LOCK_SUFFIX);
            if (!redisHandler->exists(lock)) {
                redisHandler->write(lock, "1"); // lock it
            } else {
                std::chrono::milliseconds(1);
                continue;
            }
        }

        // 3. Parse the Command
        parser->parse(line);
        parser->resetState();

        // 4. Remove the element and it's lock
        redisHandler->deleteKey(key);
        redisHandler->deleteKey(lock);

        // 5. execute timeout
        std::chrono::milliseconds(10);
    }
    return 0;
}


