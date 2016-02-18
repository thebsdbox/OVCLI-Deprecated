//
//  OVSessionID.c
//  OVCLI
//
//  Created by Daniel Finneran on 21/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include <unistd.h>

#include "OVUtils.h"
#include "OVOutput.h"
//#include "OVHttps.h"
#include "dcHttp.h"

#include "jansson.h"

extern char *OV_Version;

void setOVHeaders(char *sessionID)
{
    appendHttpHeader("Content-Type: application/json");
    appendHttpHeader(OV_Version);
    if (sessionID)
    {
        createHeader("Auth: ", sessionID);
    }
}

int identifySystem(char *oneViewAddress)
{
    // This function will iterate through the available systems and attempt to identify the system and it's version
    if (!oneViewAddress) {
        // No URL
        return 1;
    }
    //Begin Checks check

    char *httpData;
    SetHttpMethod(DCHTTPGET);
    json_t *root;
    json_error_t error;

    char urlString[256];
    createURL(urlString, oneViewAddress, "version");
    if (getenv("OV_DEBUG"))
        printf("[INFO] URL:\t %s\n", urlString);

    setOVHeaders(NULL);
    httpData = httpFunction(urlString);
    root = json_loads(httpData, 0, &error);
    free(httpData);
    json_int_t version = json_integer_value(json_object_get(root, "currentVersion"));
    // Casting from a long long to an integer (currently the versioning scheme in HPE OneView is a number in the hundreds to define major/minor
    return (int) version;
}

int stringMatch(char *string1, char *string2)
{
    while (*string1 == *string2) {
        // if both points in strings are null break out of loop
        if (*string1 == '\0' || *string2 == '\0') {
            break;
        }
        // move onto next character comparison
        string1++;
        string2++;
    }
    // Have we reached the end of the string whilst both have been the same value
    if (*string1 == '\0' && *string2 == '\0')
        return 1; // Return true (compiler)
    else
        return 0; // Return false
}

void createURL(char urlString[], char *address, char *url)
{
    snprintf(urlString, 256, "https://%s/rest/%s", address, url);
}

int writeDataToFile(const char *data, const char *filename)
{
    FILE *fp;
    char filepath[1000]; //large file path
    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, filename); // Append the filename to path
    
    fp = fopen(filepath, "w");
    if (fp) { /*file opened succesfully */
        fputs(data, fp);
        fclose(fp);
    } else {
        printf("Not logged into HP OneView host\n");
        printf("Error opening file %s \n", filepath);
        return -1;
    }
    
    return 0;
}
int writeSessionIDforHost(const char *sessionID, const char *host)
{
    return writeDataToFile(sessionID, host);
}

int writeSessionID(const char *sessionID)
{
    return writeSessionIDforHost(sessionID, sessionIDPath);
}

char *readDataFromFile(const char *file)
{
    FILE *fp;
    char buffer[1024]; //size of Session ID
    char filepath[1000]; //large file path
    char *sessionID = NULL;
    sessionID = malloc(sizeof(buffer));
    
    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, file);
    
    fp = fopen(filepath, "r");
    if (fp) { /*file opened succesfully */
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);
    } else {
        printf("Error opening file %s \n", filepath);
        free(sessionID);
        return NULL;
    }
    strcpy(sessionID, buffer);
    return sessionID;
}

char* readSessionIDforHost(const char *host)
{
    return readDataFromFile(host);
}

char* readSessionID(void)
{
    return readSessionIDforHost(sessionIDPath);
}

int ovLogin(char *argument[], char *path)
{
    char urlString[256];
    char *httpData;
    json_t *root;
    json_error_t error;
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *username = argument[3]; // Username Parameter
    char *password = argument[4]; // Password Parameter
    
    // Pack the json
    root = json_pack("{s:s, s:s}", "userName", username, "password", password);
    // dump it as a string to pass to the curl libs
    char *json_text = json_dumps(root, JSON_ENCODE_ANY);
    
    //build URL
    createURL(urlString, oneViewAddress, "login-sessions");
    
    if (getenv("OV_DEBUG"))
        printf("[INFO] URL:\t %s\n", urlString);
    // Call to HP OneView API
    setHttpData(json_text);
    setOVHeaders(NULL);
    httpData = httpFunction(urlString);
    if (getenv("OV_DEBUG"))
        printf("[INFO] JSON:\t %s\n", json_text);
    free (json_text);
    if(!httpData)
        return 1;
    
    root = json_loads(httpData, 0, &error);
    free(httpData);
    char* sessionID = (char *)json_string_value(json_object_get(root, "sessionID"));
    // Write the Session
    if (writeSessionIDforHost(sessionID, path) != 0) {
        printMessage(RED, "ERROR", "The HP OneView Session ID could not be saved");
        return 1;
    }
    if (getenv("OV_DEBUG"))
        printMessage(GREEN, "DEBUG", sessionID);
    json_decref(root);
    return 0;
    
}