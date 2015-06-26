//
//  OVSessionID.h
//  OVCLI
//
//  Created by Daniel Finneran on 21/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#ifndef __OVCLI__OVSessionID__
#define __OVCLI__OVSessionID__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define sessionIDPath "/.sessionID"

#endif /* defined(__OVCLI__OVSessionID__) */

int writeSessionID(const char *); // Write the session ID to ~/.sessionID
char* readSessionID(void); // return the session ID as a string from ~/.sessionID (returns NULL in error)

int writeSessionIDforHost(const char *sessionID, const char *host);
char* readSessionIDforHost(const char *host);