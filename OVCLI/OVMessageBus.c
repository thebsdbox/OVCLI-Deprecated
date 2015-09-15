//
//  OVMessageBus.c
//  OVCLI
//
//  Created by Daniel Finneran on 15/09/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVUtils.h"
#include "OVMessageBus.h"
#include "OVHttps.h"
#include "jansson.h"

int ovMessageBusWatch (char *sessionID, char *argument[], char *path)
{
    char urlString[256]; // Used to hold a full URL
    char *httpData;
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    json_t *root; // Contains all of the json data once processed by jansson
    json_error_t error; // Call back reference from JSON processing
    
    root = json_pack("{s:s, s:s}", "type", "RabbitMqClientCertV2", "commonName", "default");
    createURL(urlString, oneViewAddress, "certificates/client/rabbitmq");
       
    // Call to HP OneView API to Generate the keys/certs (409 ERROR if certificate exists)
    httpData = postRequestWithUrlAndDataAndHeader(urlString, json_dumps(root, JSON_ENSURE_ASCII), sessionID);
    
    if(!httpData)
        return 1;
    
    // Call to HP OneView for a client certificate and private key
    createURL(urlString, oneViewAddress, "certificates/client/rabbitmq/keypair/default");
    httpData = getRequestWithUrlAndHeader(urlString, sessionID);
    if(!httpData)
        return 1;
    // Process the raw http data and free the allocated memory
    root = json_loads(httpData, 0, &error);
    free(httpData);
    
    // Read the strings from the keys
    const char* SSLCert = json_string_value(json_object_get(root, "base64SSLCertData"));
    const char* SSLKey = json_string_value(json_object_get(root, "base64SSLKeyData"));

    char filePath[150];
    
    // Save Certificate and Key to files
    sprintf(filePath, "%s_key", path);
    writeDataToFile(SSLKey, filePath );

    sprintf(filePath, "%s_cert", path);
    writeDataToFile(SSLCert, filePath );

    // Call to HP OneView for a root Certificate Authority
    createURL(urlString, oneViewAddress, "certificates/ca");
    httpData = getRequestWithUrlAndHeader(urlString, sessionID);
    if(!httpData)
        return 1;
    
    // This doesn't return JSON, unlike the rest of the API ... WHY...
    
    // Loop through the string find the double escaped slash then step over the remaining 'n'
    int changedChar = 0;
    for(int i = 0; i <=strlen(httpData); i++) {
        if (httpData[(i + changedChar)] == '\\') {
            httpData[i] = '\n';
            changedChar++;
        } else {
            httpData[i] = httpData[i + changedChar];
        }
    }
    // Move all characters one place forward to remove the quotation
    for(int i = 0; i <=strlen(httpData); i++) {
            httpData[i] = httpData[i+1];
    }
    
    // remove the trailing quote
    httpData[strlen(httpData) -1] = '\0';

    // Save the file
    sprintf(filePath, "%s_pem", path);
    writeDataToFile(httpData, filePath);
    printf("%s", httpData);
    
    return 0;
}