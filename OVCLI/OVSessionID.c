//
//  OVSessionID.c
//  OVCLI
//
//  Created by Daniel Finneran on 21/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVSessionID.h"


int writeSessionID(const char *sessionID)
{
    FILE *fp;
    char filepath[1000]; //large file path
    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, sessionIDPath); // Append the SessionID path
    
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

char* readSessionID()
{
    FILE *fp;
    char buffer[33]; //size of Session ID
    char filepath[1000]; //large file path
    char *sessionID = NULL;
    sessionID = malloc(sizeof(buffer));

    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, sessionIDPath);

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