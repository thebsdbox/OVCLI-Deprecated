//
//  OVCopy.c
//  OVCLI
//
//  Created by Daniel Finneran on 11/08/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVCopy.h"

#include <string.h>

#include "OVUtils.h"
#include "OVCreate.h"
#include "OVHttps.h"
#include "jansson.h"


int ovCopy (char *sessionID, char *argument[])
{
    
    char urlString[256]; // Used to hold a full URL
    char path[256]; // Used to hold path to a second session file

        // Check Arguments before continuing
        if (!sessionID) {
            return 1;
        }

    
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *copyType = argument[3]; // Type of information to show
    // char *queryType = argument[4]; // Type of Query to show
    char *httpData; // Contains all fo the data returned from a http request
    json_t *root; // Contains all of the json data once processed by jansson
    json_error_t error; // Used as a passback for error data during json processing
    
    if (strstr(copyType, "NETWORKS")) {
        createURL(urlString, oneViewAddress, "ethernet-networks");

        
        // Call to HP OneView API
        httpData = getRequestWithUrlAndHeader(urlString, sessionID);
        
        if(!httpData)
            return 1;
        
        root = json_loads(httpData, 0, &error);
        
        json_t *memberArray = json_object_get(root, "members");
        if (json_array_size(memberArray) != 0) {
            size_t index;
            json_t *network = NULL;
            //char *json_text;
            json_array_foreach(memberArray, index, network) {
                const char *uri = json_string_value(json_object_get(network, "uri"));
                if (uri != NULL) {
                    if (strstr(uri, argument[4])) {
                        // Remove bits
                        //json_object_del(network, "uri");
                        json_object_del(network, "eTag");
                        json_object_del(network, "modified");
                        json_object_del(network, "created");
                        json_object_del(network, "connectionTemplateUri");
                        createURL(urlString, argument[5], "ethernet-networks");
                        sprintf(path, "/.%s_ov",argument[5]);
                        
                        sessionID = readSessionIDforHost(path);
                        httpData = postRequestWithUrlAndDataAndHeader(urlString, json_dumps(network, JSON_ENSURE_ASCII), sessionID);
                        
                        if(!httpData)
                            return 1;
                        
                    }
                }
            }
        }
        
    } else if (strstr(copyType, "SERVER-PROFILES")) {
        
        createURL(urlString, oneViewAddress, "server-profiles");
        // Call to HP OneView API
        httpData = getRequestWithUrlAndHeader(urlString, sessionID);
        
        if(!httpData)
            return 1;
        
        root = json_loads(httpData, 0, &error);
        
        // Find Server Profile first
        //int fieldCount = argc -5; //argv[0] is the path to the program
        json_t *memberArray = json_object_get(root, "members");
        if (json_array_size(memberArray) != 0) {
            size_t index;
            json_t *serverProfile = NULL;
            //char *json_text;
            json_array_foreach(memberArray, index, serverProfile) {
                const char *uri = json_string_value(json_object_get(serverProfile, "uri"));
                if (uri != NULL) {
                    if (strstr(uri, argument[4])) {
                        
                        json_object_del(serverProfile, "uri");
                        json_object_del(serverProfile, "serialNumber");
                        json_object_del(serverProfile, "uuid");
                        json_object_del(serverProfile, "taskUri");
                        json_object_del(serverProfile, "status");
                        json_object_del(serverProfile, "inProgress");
                        json_object_del(serverProfile, "modified");
                        json_object_del(serverProfile, "eTag");
                        json_object_del(serverProfile, "created");
                        json_object_del(serverProfile, "serverHardwareUri");
                        json_object_del(serverProfile, "enclosureBay");
                        json_object_del(serverProfile, "enclosureUri");
                        
                        //json_object_del(serverProfile, "connections");
                        json_t *connections, *connectionsArray= json_object_get(serverProfile, "connections");
                        json_array_foreach(connectionsArray, index, connections) {
                            json_object_del(connections, "mac");
                            json_object_del(connections, "wwnn");
                            json_object_del(connections, "wwpn");
                        }
                        
                        //int profileCount = atoi(argv[5]);
                        //if (profileCount != 0){
                        //  char name[100];
                        
                        //char profileName[100];
                        //strcpy(profileName, json_string_value(json_object_get(serverProfile, "name")));
                        //for (int i =0; i <profileCount; i++) {
                        // sprintf(name, "%s_%d",profileName, i);
                        //  printf("%s\n", name);
                        // json_string_set( json_object_get(serverProfile, "name"), name);
                        json_string_set(json_object_get(serverProfile, "enclosureGroupUri"), argument[6]);
                        json_string_set(json_object_get(serverProfile, "serverHardwareTypeUri"), argument[7]);
                        
                        createURL(urlString, argument[5], "ethernet-networks");
                        sprintf(path, "/.%s_ov",argument[5]);
                        
                        sessionID = readSessionIDforHost(path);
                        httpData = postRequestWithUrlAndDataAndHeader(urlString, json_dumps(serverProfile, JSON_ENSURE_ASCII), sessionID);
                        
                        if(!httpData)
                            return 1;
                        
                        
                        
                    }
                    
                }
            }
        }

    }
    return 0;
}

void ovCopyPrintHelp()
{
    printf("\n OVCLI xxx.xxx.xxx.xxx COPY <TYPE> <...> <...>");
    printf("\n <TYPE>");
    printf("\n\t NETWORKS <NETWORK-URI> <DESTINATION ONEVIEW>");
    printf("\n\t SERVER-PROFILES <SERVER-PROFILE URI> <DESTINATION ONEVIEW>\n");

}