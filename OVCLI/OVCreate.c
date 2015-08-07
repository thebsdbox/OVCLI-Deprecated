//
//  OVCreate.c
//  OVCLI
//
//  Created by Daniel Finneran on 05/08/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//
#include <string.h>

#include "OVUtils.h"
#include "OVCreate.h"
#include "OVHttps.h"
#include "jansson.h"


int ovCreate (char *sessionID, char *argument[])
{
    char urlString[256];
    
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *createType = argument[3]; // Type of information to create
    char *httpData;
    json_t *root;
    
    if (strstr(createType, "SERVER-PROFILES")) {
        createURL(urlString, oneViewAddress, "server-profiles");

        // This will just create a simple empty profile and apply it to an enclosure group
        root = json_pack("{s:s, s:s, s:s, s:s}","type", "ServerProfileV4", "name", argument[4], "serverHardwareTypeUri", argument[5], "enclosureGroupUri", argument[6]);
        char *json_text = json_dumps(root, JSON_ENSURE_ASCII);

        httpData = postRequestWithUrlAndDataAndHeader(urlString, json_text, sessionID);
        
        if(!httpData)
            return 1;
        
        free(json_text);

    } else if (strstr(argument[3], "SERVER-HARDWARE-TYPES")) {
        createURL(urlString, oneViewAddress, "server-hardware-types");

    } else if (strstr(argument[3], "ENCLOSURE-GROUPS")) {
        createURL(urlString, oneViewAddress, "enclosure-groups");

        // Comprehensive json needs creating for the Enlcosure Group type, Including details for all eight interconnect Bays
        root = json_pack("{s:s, s:s, s:s, s:[{s:i, s:s}, \
                         {s:i, s:s}, {s:i, s:s}, \
                         {s:i, s:s}, {s:i, s:s}, \
                         {s:i, s:s}, {s:i, s:s}, \
                         {s:i, s:s},]}",         \
                         "type", "EnclosureGroupV2",
                         "name", argument[4], \
                         "stackingMode", "Enclosure", \
                         "interconnectBayMappings", "interconnectBay", 1, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 2, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 3, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 4, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 5, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 6, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 7, "logicalInterconnectGroupUri", argument[5], "interconnectBay", 8, "logicalInterconnectGroupUri", argument[5]);
        char *json_text = json_dumps(root, JSON_ENSURE_ASCII); //4 is close to a tab

        
        httpData = postRequestWithUrlAndDataAndHeader(urlString, json_text, sessionID);
        free(json_text);
        json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
        
        //printf("[DEBUG] JSON:\t %s\n", json_text);
        
        free(json_text);
    } else if (strstr(argument[3], "NETWORKS")) {
        char *vlan = argument[5];
        char *purpose = argument[6];
        char *name = argument[4];
        createURL(urlString, oneViewAddress, "ethernet-networks");
        root = json_pack("{s:s, s:s, s:s, s:b, s:b, s:s, s:s, s:n}", \
                         "vlanId", vlan,                      \
                         "purpose" , purpose,                    \
                         "name" , name,                       \
                         "smartLink" , JSON_TRUE,                   \
                         "privateNetwork" , JSON_FALSE,              \
                         "ethernetNetworkType" , "Tagged",           \
                         "type" , "ethernet-networkV2" ,    \
                         "connectionTemplateUri" , "");
        char *json_text = json_dumps(root, JSON_ENSURE_ASCII); //4 is close to a tab
        
        
        httpData = postRequestWithUrlAndDataAndHeader(urlString, json_text, sessionID);
        free(json_text);
    } else {
        printCreateHelp();
    }
     return  0;
}

void printCreateHelp()
{
    // Display the help (to be cleared at a later date
    printf("\n SHOW COMMANDS\n------------\n");
    printf(" SERVER-PROFILES - List server profiles\n");
    printf(" SERVER-HARDWARE - list detected physical hardware\n");
    printf(" SERVER-HARDWARE-TYPES - List discovered hardware types\n");
    printf(" ENCLOSURE-GROUPS - List defined enclosure groups\n");
    printf(" NETWORKS - List defined networks\n");
    printf(" INTERCONNECTS - List uplinks\n\n");
}