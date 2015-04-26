/*
 *  emit.h
 *
 *  Defines an interface that handles broadcasting messages to the user when invoked from
 *  from operational modules
 *
 *  Created by Ryan Faulkner on 2015-03-29
 *  Copyright (c) 2015. All rights reserved.
 */

#ifndef _emit_h
#define _emit_h

#include <iostream>
#include <string>

void emitCLIError(std::string message) { std::cout << std::string("ERR: ") + message << std::endl; }

void emitCLIWarning(std::string message) { std::cout << std::string("WARN: ") + message << std::endl; }

void emitCLINote(std::string message) { std::cout << std::string("NOTE: ") + message << std::endl; }

void emitCLIGeneric(std::string s) { std::cout << s << std::endl; }

#endif
