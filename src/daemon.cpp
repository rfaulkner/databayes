

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

#define DBY_CMD_QUEUE_PREFIX "dby_command_queue_"

using namespace std;


/**
 * This method handles fetching the next item to be
 *
 * TODO - implement
 */
std::string getNextQueueKey(RedisHandler& rh) {
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

        // TODO - implement locking function

        // 1. Fetch next command off the queue
        this->redisHandler->connect();
        std::string key = getNextQueueKey(*redisHandler);
        if (this->redisHandler->exists(key))
            line = this->redisHandler->read(key);

        // TODO - 2. LOCK KEY

        // 3. Parse the Command
        parser->parse(line);
        parser->resetState();

        // 4. execute timeout
        this->redisHandler->deleteKey(key)

        // 5. execute timeout
        std::chrono::milliseconds(10)
    }
    return 0;
}


