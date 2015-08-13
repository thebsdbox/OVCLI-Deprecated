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
    
    
    if (strstr(argv[3], "NETWORKS")) {
        snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "ethernet-networks");
        
        // Call to HP OneView API
        httpData = getRequestWithUrlAndHeader(url, sessionID);
        
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
                    if (strstr(uri, argv[4])) {
                        // Remove bits
                        //json_object_del(network, "uri");
                        json_object_del(network, "eTag");
                        json_object_del(network, "modified");
                        json_object_del(network, "created");
                        json_object_del(network, "connectionTemplateUri");
                        
                        snprintf(url, URL_SIZE, URL_FORMAT, argv[5], "ethernet-networks");
                        sprintf(path, "/.%s_ov",argv[5]);
                        
                        sessionID = readSessionIDforHost(path);
                        httpData = postRequestWithUrlAndDataAndHeader(url, json_dumps(network, JSON_ENSURE_ASCII), sessionID);
                        
                        if(!httpData)
                            return 1;
                        
                    }
                }
            }
        }
        
    } else if (strstr(argv[3], "SERVER-PROFILES")) {
        snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-profiles");
        
        // Call to HP OneView API
        httpData = getRequestWithUrlAndHeader(url, sessionID);
        
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
                    if (strstr(uri, argv[4])) {
                        
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
                        json_string_set(json_object_get(serverProfile, "enclosureGroupUri"), argv[6]);
                        json_string_set(json_object_get(serverProfile, "serverHardwareTypeUri"), argv[7]);
                        
                        snprintf(url, URL_SIZE, URL_FORMAT, argv[5], "server-profiles");
                        sprintf(path, "/.%s_ov",argv[5]);
                        
                        sessionID = readSessionIDforHost(path);
                        httpData = postRequestWithUrlAndDataAndHeader(url, json_dumps(serverProfile, JSON_ENSURE_ASCII), sessionID);
                        
                        if(!httpData)
                            return 1;
                        
                        
                        
                    }
                    
                }
            }
        }

}

void printCopyHelp()
{
    
}