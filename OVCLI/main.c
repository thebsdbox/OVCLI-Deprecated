/*
 * A command line interface for HP OneView
 *
 * Daniel Finneran (HP) Daniel.jam.finneran@hp.com 20/05/2015
 *
 * IP Addressing check code taken from http://www.geeksforgeeks.org/program-to-validate-an-ip-address/
 */

#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <ctype.h>


// JSON
#include "jansson.h"



#include "OVUtils.h"
#include "OVHttps.h"
#include "OVOutput.h" // Contains the debug option

#include "OVShow.h"
#include "OVCreate.h"
#include "OVCopy.h"
#include "OVMessageBus.h"




#define URL_FORMAT   "https://%s/rest/%s"
#define URL_SIZE     256
#define DELIM "."

int debug;

/* return 1 if string contain only digits, else return 0 */
int valid_digit(char *ip_str)
{
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return 0;
    }
    return 1;
}

/* return 1 if IP string is valid, else return 0 */
int is_valid_ip(char *ip_str)
{
    // Additional code to copy the string so the original stays intact
    char *ptr, *tempString = calloc(strlen(ip_str)+1, sizeof(char));
    strcpy(tempString, ip_str);
    int num, dots = 0;
    
    if (tempString == NULL)
        return 0;
    
    // See following link for strtok()
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strtok_r.html
    ptr = strtok(tempString, DELIM);
    
    if (ptr == NULL)
        return 0;
    
    while (ptr) {
        
        /* after parsing string, it must contain only digits */
        if (!valid_digit(ptr))
            return 0;
        
        num = atoi(ptr);
        
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL)
                ++dots;
        } else
            return 0;
    }
    free(tempString);
    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return 0;
    return 1;
}



int main(int argc, char *argv[])
{
    char *oneViewAddress;
    
    // Check for Debug Mode
    if (getenv("OV_DEBUG")) {
        debug = 1; // debug mode enabled
    } else {
        debug = 0; // debug mode disabled
    }

    
    // Peform an initial check to see what parameters have been passed
    char path[100];
    if (argc >1) {
        oneViewAddress = argv[1];
        if (is_valid_ip(oneViewAddress)) {
            // Create a string based upon the path to the sessionID
            sprintf(path, "/.%s_ov",oneViewAddress);
        } else {
            printMessage(YELLOW, "DEBUG", "Invalid IP Address");
            return 1;
        }
    }
    if(argc < 3)
    {
        printMessage(YELLOW, "DEBUG", "No parameters passed");
        fprintf(stderr, "usage: %s ADDRESS COMMAND <parameters>\n", argv[0]);
        // Somewhat over the top logo
        fprintf(stderr, " \
                _   _ ____     ___           __     ___\n\
                | | | |  _ \\   / _ \\ _ __   __\\ \\   / (_) _____      __\n\
                | |_| | |_) | | | | | '_ \\ / _ \\ \\ / /| |/ _ \\ \\ /\\ / /\n\
                |  _  |  __/  | |_| | | | |  __/\\ V / | |  __/\\ V  V /\n\
                |_| |_|_|      \\___/|_| |_|\\___| \\_/  |_|\\___| \\_/\\_/  \n\n");
        //Print the relevant helps
        ovCreatePrintHelp();
        ovShowPrintHelp();
        ovCopyPrintHelp();
        return 1;
    }
 
    char url[URL_SIZE];
    char *httpData;
    json_t *root = NULL;
    json_error_t error;

    // Determine the action to be executed

    if (stringMatch(argv[2], "LOGIN")) {
        // Login to HP OneView
        ovLogin(argv, path);
        return 0;
    } else if (strstr(argv[2], "SHOW")) {
        // Show/Query information from HP OneView
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID\n");
            return 1;
        }
        ovShow(sessionID, argc, argv);
        return 0; // return sucess

    } else if (strstr(argv[2], "CREATE")) {
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID\n");
            return 1;
        }
        ovCreate(sessionID, argv);
        return 0; // Return success
    } else if (strstr(argv[2], "COPY")) {
    
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID\n");
            return 1;
        }
        ovCopy(sessionID, argv);
        return 0; //return sucess
        
        // Debug OVID outpu
        //printf("[DEBUG] OVID:\t  %s\n",sessionID);
        /*
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
*/
    
        
        
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
    } else if (strstr(argv[2], "MESSAGEBUS")) {
        char *sessionID = readSessionIDforHost(path);
        if (!sessionID) {
            printf("[ERROR] No session ID\n");
            return 1;
        } else if (argc < 4) {
            printf("[ERROR] Incorrect usage\n");
            return 1;
        }
        if (strstr(argv[3], "GENERATE")) {
            ovMsgBusCertGenerate(sessionID, argv);
        }
        
        if (strstr(argv[3], "CERT")) {
           ovMsgBusCertDownload(sessionID, argv, path);
        }
        
        if (stringMatch(argv[3], "LISTEN")) {
            ovMsgBusListen(argv, path);
        }
    }

    return 0;
}
