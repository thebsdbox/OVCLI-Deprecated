//
//  OVSessionID.c
//  OVCLI
//
//  Created by Daniel Finneran on 21/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVSessionID.h"




int writeSessionIDforHost(const char *sessionID, const char *host)
{
    FILE *fp;
    char filepath[1000]; //large file path
    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, host); // Append the SessionID path
    
    fp = fopen(filepath, "w");
    if (fp) { /*file opened succesfully */
        fputs(sessionID, fp);
        fclose(fp);
    } else {
        printf("Error opening file %s \n", filepath);
        return -1;
    }
    
    return 0;
}

int writeSessionID(const char *sessionID)
{
    return writeSessionIDforHost(sessionID, sessionIDPath);
}

char* readSessionIDforHost(const char *host)
{
    FILE *fp;
    char buffer[33]; //size of Session ID
    char filepath[1000]; //large file path
    char *sessionID = NULL;
    sessionID = malloc(sizeof(buffer));

    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, host);

    fp = fopen(filepath, "r");
    if (fp) { /*file opened succesfully */
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);
    } else {
        printf("Error opening file %s \n", filepath);
        return NULL;
    }
    strcpy(sessionID, buffer);
    return sessionID;
}

char* readSessionID(void)
{
    return readSessionIDforHost(sessionIDPath);
}