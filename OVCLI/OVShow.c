//
//  OVShow.c
//  OVCLI
//
//  Created by Daniel Finneran on 04/08/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

// OneView headers
#include "OVShow.h"
#include "OVHttps.h"
#include "OVUtils.h"
// Json Processing
#include "jansson.h"
// C libraries
#include <stdlib.h>
#include <string.h>

/*
 
 ovShow Method takes a show type (i.e. server profiles or networks), and a show method (such as JSON)
 
 */

int ovShow(char *sessionID, int argumentCount, char *argument[])
{
    char urlString[256];

    if (argumentCount >=4) {
    // Check Arguments before continuing
        if (!sessionID) {
            return 1;
        }
    } else {
        return 1;
    }
    
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *showType = argument[3]; // Type of information to show
    char *queryType = argument[4]; // Type of Query to show
    char *httpData; // Contains all fo the data returned from a http request
    json_t *root; // Contains all of the json data once processed by jansson
    json_error_t error; // Used as a passback for error data during json processing
    
    // Determing the Show Type and construct the correct URL
        
    if (strstr(showType, "SERVER-PROFILES")) {
        createURL(urlString, oneViewAddress, "server-profiles");
    } else if (strstr(showType, "SERVER-HARDWARE-TYPES")) {
        createURL(urlString, oneViewAddress, "server-hardware-types");
    } else if (strstr(showType, "SERVER-HARDWARE")) {
        createURL(urlString, oneViewAddress, "server-hardware");
    } else if (strstr(showType, "ENCLOSURE-GROUPS")) {
        createURL(urlString, oneViewAddress, "enclosure-groups");
    } else if (strstr(showType, "NETWORKS")) {
        createURL(urlString, oneViewAddress, "ethernet-networks");
    } else if (strstr(showType, "NETWORK-SETS")) {
        createURL(urlString, oneViewAddress, "network-sets");
    } else if (strstr(showType, "INTERCONNECT-GROUPS")) {
        createURL(urlString, oneViewAddress, "logical-interconnect-groups");
    } else if (strstr(showType, "INTERCONNECTS")) {
        createURL(urlString, oneViewAddress, "interconnects");
    } else {
            // Display the help (to be cleared at a later date
            printf("\n SHOW COMMANDS\n------------\n");
            printf(" SERVER-PROFILES - List server profiles\n");
            printf(" SERVER-HARDWARE - list detected physical hardware\n");
            printf(" SERVER-HARDWARE-TYPES - List discovered hardware types\n");
            printf(" ENCLOSURE-GROUPS - List defined enclosure groups\n");
            printf(" NETWORKS - List defined networks\n");
            printf(" INTERCONNECTS - List uplinks\n\n");
            return 1;
    }
    
    // If a query has been submitted speak to OneView and process the results
    if (queryType) {
        
        // Pass the URL and sessionID to HP OneView and return the response
        httpData = getRequestWithUrlAndHeader(urlString, sessionID);
        
        // If response is empty fail
        if(!httpData)
            return 1;
        
        // Process the raw http data and free the allocated memory
        root = json_loads(httpData, 0, &error);
        free(httpData);
        
        
        
        if (strstr(queryType, "RAW")) {
            char *json_text = json_dumps(root, JSON_ENSURE_ASCII);
            printf("%s\n", json_text);
            free(json_text);
            return 0;
        } else if (strstr(queryType, "PRETTY")) {
            char *json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
            printf("%s\n", json_text);
            free(json_text);
            return 0;
        } else if (strstr(queryType, "URI")) {
            // will return URI and Name
            json_t *memberArray = json_object_get(root, "members");
            if (json_array_size(memberArray) != 0) {
                size_t index;
                json_t *value;
                json_array_foreach(memberArray, index, value) {
                    const char *uri = json_string_value(json_object_get(value, "uri"));
                    const char *name = json_string_value(json_object_get(value, "name"));
                    printf("%s \t %s\n", uri, name);
                }
            }
            return 0;
        } else if (strstr(queryType, "FIELDS")) {
            
            //int fieldCount = argc -5; //argv[0] is the path to the program
            json_t *memberArray = json_object_get(root, "members");
            if (json_array_size(memberArray) != 0) {
                size_t index;
                json_t *value;
                json_array_foreach(memberArray, index, value) {
                    for (int i = 5; i< argumentCount; i++) {
                        const char *fieldObject = json_string_value(json_object_get(value, argument[i]));
                        if (fieldObject == NULL) {
                            printf("\"\" \t"); // If NULL value is returned or Field not found then "" is returned
                        } else
                            printf("%s \t", fieldObject);
                    }
                    printf(" \n"); // New line
                }
            }
            return 0;
            
        }
    }
    
    // Either a unknown query or no query was submitted so display help
    printf("\n\nPlease enter output:\n");
    printf("RAW: ASCII ENCODED RAW JSON\n");
    printf("PRETTY: Parsed and Indented JSON\n");
    printf("URI: Lists NAME and URI\n");
    printf("\n\n");

    // Should probably re-address the return value at a later time
    return 0;
}

void ovShowPrintHelp()
{
    printf("\n OVCLI xxx.xxx.xxx.xxx SHOW <TYPE> <OUTPUT>");
    printf("\n <OUTPUT>");
    printf("\n\t RAW - raw JSON");
    printf("\n\t PRETTY - readable JSON");
    printf("\n\t URI - prints NAME and URI fields");
    printf("\n\t FIELDS <FIELD 1> <FIELD 2> <FIELD 3>");
    printf("\n <TYPE>");
    printf("\n\t NETWORKS <OUTPUT>");
    printf("\n\t SERVER-PROFILES <OUTPUT>");
}