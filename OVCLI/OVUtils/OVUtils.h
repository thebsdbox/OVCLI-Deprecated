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

// Write Session ID strings to Home Directory
int writeSessionID(const char *); // Write the session ID to ~/.sessionID
int writeSessionIDforHost(const char *sessionID, const char *host);
int writeDataToFile(const char *data, const char *filename);


//Read the Session IDs stored in the Home Directory
char* readSessionIDforHost(const char *host);
char* readSessionID(void); // return the session ID as a string from ~/.sessionID (returns NULL in error)
char *readDataFromFile(const char *file);

// Login function
int ovLogin(char *argument[], char *path);

// URL Generator for restAPI paths..
void createURL(char urlString[], char *address, char *url);

int stringMatch(char *string1, char *string2);
// Helper function to automate the headers
void setOVHeaders(char *sessionID);
