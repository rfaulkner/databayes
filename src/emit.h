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

void emitCLIError(std::string, bool isDebug = false) {}
void emitCLIWarning(std::string, bool isDebug = false) {}
void emitCLINote(std::string, bool isDebug = false) {}

#endif
