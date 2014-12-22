/*
 *  socket.h
 *
 *  Defines the Databayes socket interface that handles forwarding HTTP requests to the system.  Exposes a
 *  RESTful API.
 *
 *  Created by Ryan Faulkner on 2014-12-21
 *  Copyright (c) 2014. All rights reserved.
 */


#ifndef _socket_h
#define _socket_h

#include <string>
#include <vector>
#include "index.h"
#include <json/json.h>

#define CALL_REL 1
#define CALL_ADD_REL 1

using namespace std;

class DataBayesSocket {

    IndexHandler* indexHandler;
    int action_state;           // Stores the action state of the current request

    Json::Value parseHeader(/** Session object */);
    Json::Value parseQueryParams(std::string);
    Json::Value parseAction(Json::Value);

    void handleResponse(Json::Value);       // input is the query result, handles sending back the response to sender

public:

    DataBayesSocket() { this->indexHandler = new IndexHandler(); }
    int listen(int);    // defines the listener and port
};


#endif
