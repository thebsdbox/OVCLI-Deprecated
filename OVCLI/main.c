/*
 * A command line interface for HP OneView
 *
 * Daniel Finneran (HP) Daniel.jam.finneran@hp.com 20/05/2015
 */

#include <stdlib.h>
#include <string.h>

#include "jansson.h"


#include "OVSessionID.h"
#include "OVHttps.h"
#include "OVOutput.h" // Contains the debug option

#include "OVShow.h"


#define URL_FORMAT   "https://%s/rest/%s"
#define URL_SIZE     256

int debug;

int main(int argc, char *argv[])
{
    
    if (getenv("OV_DEBUG")) {
        debug = 1; // debug mode enabled
    } else {
        debug = 0; // debug mode disabled
    }

    char url[URL_SIZE];
    
    char *httpData;
    json_t *root;
    json_error_t error;
    char path[100];
    if (argc >1) {
        sprintf(path, "/.%s_ov",argv[1]);
    }
    if(argc < 3)
    {
        printMessage(YELLOW, "DEBUG", "No parameters passed");
        fprintf(stderr, "usage: %s ADDRESS COMMAND <parameters>\n\n", argv[0]);
        fprintf(stderr, "HP OneView CLI Utility 2015.\n\n");
        return 2;
    }

    if (strstr(argv[2], "LOGIN")) {

        // Pack the json
        root = json_pack("{s:s, s:s}", "userName", argv[3], "password", argv[4]);
        // dump it as a string to pass to the curl libs
        char *json_text = json_dumps(root, JSON_ENCODE_ANY);
        //build URL
        snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "login-sessions");
        if (getenv("OV_DEBUG"))
            printf("[INFO] URL:\t %s\n", url);
        // Call to HP OneView API
        httpData = postRequestWithUrlAndData(url, json_text);
        if (getenv("OV_DEBUG"))
            printf("[INFO] JSON:\t %s\n", json_text);
        free (json_text);
        if(!httpData)
            return 1;
        
        root = json_loads(httpData, 0, &error);
        free(httpData);
        const char* sessionID = json_string_value(json_object_get(root, "sessionID"));
        //

        
        // Write the Session
        if (writeSessionIDforHost(sessionID, path) != 0) {
            printMessage(RED, "ERROR", "The HP OneView Session ID could not be saved");
            return 1;
        }
        printf("[DEBUG] OVID:\t  %s\n",sessionID);
        return 0;
        
    } else if (strstr(argv[2], "SHOW")) {
        
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID");
            return 1;
        }
        
        ovShow(sessionID, argc, argv);

        
        //printf("[DEBUG] OVID:\t  %s\n",sessionID);
        if (argc >=4) {
        if (strstr(argv[3], "SERVER-PROFILES")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-profiles");
        } else if (strstr(argv[3], "SERVER-HARDWARE-TYPES")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-hardware-types");
        } else if (strstr(argv[3], "SERVER-HARDWARE")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-hardware");
        } else if (strstr(argv[3], "ENCLOSURE-GROUPS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "enclosure-groups");
        } else if (strstr(argv[3], "NETWORKS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "ethernet-networks");
        } else if (strstr(argv[3], "NETWORK-SETS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "network-sets");
        } else if (strstr(argv[3], "INTERCONNECT-GROUPS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "logical-interconnect-groups");
        } else if (strstr(argv[3], "INTERCONNECTS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "interconnects");
        }} else {
            printf("\n SHOW COMMANDS\n------------\n");
            printf(" SERVER-PROFILES - List server profiles\n");
            printf(" SERVER-HARDWARE - list detected physical hardware\n");
            printf(" SERVER-HARDWARE-TYPES - List discovered hardware types\n");
            printf(" ENCLOSURE-GROUPS - List defined enclosure groups\n");
            printf(" NETWORKS - List defined networks\n");
            printf(" INTERCONNECTS - List uplinks\n\n");
            return 1;
        }
        
        
        // DISPLAY URL
        //printf("[DEBUG] URL:\t %s\n", url);
        // Call to HP OneView API
        httpData = getRequestWithUrlAndHeader(url, sessionID);
        
        if(!httpData)
            return 1;
        
        //printf("[DEBUG] JSON: %s\n", httpData);
        root = json_loads(httpData, 0, &error);
        free(httpData);
        if (argv[4] == NULL) {
            printf("\n\nPlease enter output:\n");
            printf("RAW: ASCII ENCODED RAW JSON\n");
            printf("PRETTY: Parsed and Indented JSON\n");
            printf("URI: Lists NAME and URI\n");
            printf("\n\n");
            return 1;
        }
        if (strstr(argv[4], "RAW")) {
            char *json_text = json_dumps(root, JSON_ENSURE_ASCII); //4 is close to a tab
            printf("%s\n", json_text);
            free(json_text);
        
        } else if (strstr(argv[4], "PRETTY")) {
            char *json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
            printf("%s\n", json_text);
            free(json_text);
        } else if (strstr(argv[4], "URI")) {
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
        } else if (strstr(argv[4], "FIELDS")) {

            //int fieldCount = argc -5; //argv[0] is the path to the program
            json_t *memberArray = json_object_get(root, "members");
            if (json_array_size(memberArray) != 0) {
                size_t index;
                json_t *value;
                json_array_foreach(memberArray, index, value) {
                    for (int i = 5; i< argc; i++) {
                        const char *fieldObject = json_string_value(json_object_get(value, argv[i]));
                        if (fieldObject == NULL) {
                            printf("\"\" \t"); // If NULL value is returned or Field not found then "" is returned
                        } else
                            printf("%s \t", fieldObject);
                    }
                    printf(" \n"); // New line
                }
            }
        }

        
        return 0;

    } else if (strstr(argv[2], "CREATE")) {
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID");
            return 1;
        }
        
        // Debug output
        //printf("[DEBUG] OVID:\t  %s\n",sessionID);
        
        
        if (strstr(argv[3], "SERVER-PROFILES")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-profiles");
            root = json_pack("{s:s, s:s, s:s, s:s}","type", "ServerProfileV4", "name", argv[4], "serverHardwareTypeUri", argv[5], "enclosureGroupUri", argv[6]);
            char *json_text = json_dumps(root, JSON_ENSURE_ASCII); //4 is close to a tab
            
            // Debug output
            //printf("[DEBUG] URL:\t %s\n", url);
            // Call to HP OneView API
            
            httpData = postRequestWithUrlAndDataAndHeader(url, json_text, sessionID);
            
            if(!httpData)
                return 1;
            
            free(json_text);
            json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
            
            // More Debug output
            //printf("[DEBUG] JSON:\t %s\n", json_text);
            
            free(json_text);
        } else if (strstr(argv[3], "SERVER-HARDWARE-TYPES")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-hardware-types");
        } else if (strstr(argv[3], "ENCLOSURE-GROUPS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "enclosure-groups");
            // Comprehensive json needs creating for the Enlcosure Group type, Including details for all eight interconnect Bays
            root = json_pack("{s:s, s:s, s:s, s:[{s:i, s:s}, \
                             {s:i, s:s}, {s:i, s:s}, \
                             {s:i, s:s}, {s:i, s:s}, \
                             {s:i, s:s}, {s:i, s:s}, \
                             {s:i, s:s},]}","type", "EnclosureGroupV2", "name", argv[4], "stackingMode", "Enclosure", "interconnectBayMappings", "interconnectBay", 1, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 2, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 3, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 4, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 5, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 6, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 7, "logicalInterconnectGroupUri", argv[5], "interconnectBay", 8, "logicalInterconnectGroupUri", argv[5]);
            char *json_text = json_dumps(root, JSON_ENSURE_ASCII); //4 is close to a tab
            //printf("[DEBUG] URL:\t %s\n", url);
            // Call to HP OneView API
            httpData = postRequestWithUrlAndDataAndHeader(url, json_text, sessionID);
            free(json_text);
            json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab

            //printf("[DEBUG] JSON:\t %s\n", json_text);

            free(json_text);
        } else if (strstr(argv[3], "NETWORKS")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "networks");
        }
        // Debug URL output
        //printf("[DEBUG] URL:\t %s\n", url);
        // Call to HP OneView API
        //httpData = getRequestWithUrlAndHeader(url, sessionID);
        
    } else if (strstr(argv[2], "COPY")) {
    
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID");
            return 1;
        }
        
        // Debug OVID output
        //printf("[DEBUG] OVID:\t  %s\n",sessionID);
        
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

        
        
    } else if (strstr(argv[2], "CLONE")) {
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID");
            return 1;
        }
        
        // DEBUG OVID output
        //printf("[DEBUG] OVID:\t  %s\n",sessionID);
        if (strstr(argv[3], "SERVER-PROFILES")) {
            snprintf(url, URL_SIZE, URL_FORMAT, argv[1], "server-profiles");
            
            // Call to HP OneView API
            httpData = getRequestWithUrlAndHeader(url, sessionID);
            
            if(!httpData)
                return 1;
            
            //printf("[DEBUG] JSON: %s\n", httpData);
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
                                //json_text = json_dumps(serverProfile, JSON_INDENT(4)); //4 is close to a tab
                                //printf("%s\n", json_text);
                                //
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
                                
                                int profileCount = atoi(argv[5]);
                                if (profileCount != 0){
                                    char name[100];
                                    //strcat(name, json_string_value(json_object_get(serverProfile, "name")));
                                    //strcat(name, )
                                    //const char *profileName = json_string_value(json_object_get(serverProfile, "name"));
                                    char profileName[100];
                                    strcpy(profileName, json_string_value(json_object_get(serverProfile, "name")));
                                    for (int i =0; i <profileCount; i++) {
                                        sprintf(name, "%s_%d",profileName, i);
                                        printf("%s\n", name);
                                        json_string_set( json_object_get(serverProfile, "name"), name);
                                        httpData = postRequestWithUrlAndDataAndHeader(url, json_dumps(serverProfile, JSON_ENSURE_ASCII), sessionID);
                                        
                                        if(!httpData)
                                            return 1;
                                    }
                                }
                                
                                //printf("%s\n", json_string_value(json_object_get(serverProfile, "name")));
                                //json_dumps(root, JSON_ENSURE_ASCII)
                                /*
                                httpData = postRequestWithUrlAndDataAndHeader(url, json_dumps(serverProfile, JSON_ENSURE_ASCII), sessionID);
                                
                                if(!httpData)
                                    return 1;
                                //free(json_text);
                                json_text = json_dumps(serverProfile, JSON_INDENT(4)); //4 is close to a tab
                                printf("%s\n", json_text);
                                free(json_text);
*/
                            }
                            
                        }
                }
            }
            
        }
    }
    
    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
    }


    json_decref(root);
    return 0;
}
