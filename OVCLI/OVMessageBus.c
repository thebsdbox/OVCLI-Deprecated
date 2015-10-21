//
//  OVMessageBus.c
//  OVCLI
//
//  Created by Daniel Finneran on 15/09/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//
// URLs looked through:
// https://github.com/luvit/openssl/blob/master/openssl/demos/pkcs12/pkwrite.c
// https://github.com/alanxz/rabbitmq-c/blob/master/examples/amqps_consumer.c
// https://github.com/alanxz/rabbitmq-c/blob/master/examples/utils.c
// http://h17007.www1.hp.com/docs/enterprise/servers/oneview1.2/cic-rest/en/content/s_using-SCMB-ci.html
//
//

#include "OVMessageBus.h"

// OneView Headers
#include "OVUtils.h"
#include "OVHttps.h"
#include "OVMessageBusUtils.h"

// JSON processing Header
#include "jansson.h"

// Standard Libraries
#include <ctype.h>
#include <stdlib.h>

// RabbitMQ
#include "amqp_ssl_socket.h"
#include "amqp_framing.h"

// Influx DB
#include "libinfluxdb.h"

uint64_t now_microseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) tv.tv_sec * 1000000 + (uint64_t) tv.tv_usec;
}

int ovMsgBusCertGenerate(char *sessionID, char *argument[])
{
    char urlString[256]; // Used to hold a full URL
    char *httpData;
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    json_t *root; // Contains all of the json data once processed by jansson
    
    root = json_pack("{s:s, s:s}", "type", "RabbitMqClientCertV2", "commonName", "default");
    createURL(urlString, oneViewAddress, "certificates/client/rabbitmq");
    
    // Call to HP OneView API to Generate the keys/certs (409 ERROR if certificate exists)
    httpData = postRequestWithUrlAndDataAndHeader(urlString, json_dumps(root, JSON_ENSURE_ASCII), sessionID);
    
    if(!httpData)
        return 1;
    
    return 0;
}

int ovMsgBusCertDownload(char *sessionID, char *argument[], char *path)
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

    sprintf(filePath, "%s_pem", path);
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
    sprintf(filePath, "%s_cacert", path);
    writeDataToFile(httpData, filePath);
    
    return 0;
}

int ovMetricMsgBusSetConfig(char *sessionID, char *argument[])
{
    char urlString[256]; // Used to hold a full URL
    char *httpData;
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *type = argument[5];  //Device
    char *interval = argument[6]; // Sample rate
    char *frequency = argument[7]; // Frequency of updates
    
    if (!type) {
        printf("No argument given for device to monitor e.g. /rest/enclosures\n");
        return -1;
    }
    
    if (!interval) {
        printf("No argument given for interval for sample data e.g. 300 (300 sec/5min)\n");
        return -1;
    }
    
    if (!frequency) {
        printf("No argument fiven for frequency for results e.g. 3600 (3600 sec/1hour)\n");
        return -1;
    }
    
    json_t *root; // Contains all of the json data once processed by jansson
    
    root = json_pack("{s:[{s:s,s:s,s:s}]}", "sourceTypeList", "sourceType" ,type, "sampleIntervalInSeconds",  interval, "frequencyOfRelayInSeconds", frequency);
    createURL(urlString, oneViewAddress, "metrics/configuration");
    // Call to HP OneView API to Generate the keys/certs (409 ERROR if certificate exists)
    httpData = putRequestWithURLAndDataAndHeader(urlString, json_dumps(root, JSON_ENSURE_ASCII), sessionID);
    json_decref(root);
    return 0;
}

int ovMetricMsgBusGetSettings(char *sessionID, char *argument[])
{
    char urlString[256]; // Used to hold a full URL
    char *httpData;
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *type = argument[4];
    if (type) {
        if (stringMatch(type, "CAPABILITY")) {
            type = "metrics/capability";
        } else if (stringMatch(type, "GETCONFIG")) {
            type = "metrics/configuration";
        } else if (stringMatch(type, "SETCONFIG")) {
            return ovMetricMsgBusSetConfig(sessionID, argument);
        } else {
            printf ("No argument given for Settings\n");
            return -1;
        }
        
    }
    json_t *root; // Contains all of the json data once processed by jansson
    json_error_t error; // Call back reference from JSON processing
       
    // Call to HP OneView for a client certificate and private key
    createURL(urlString, oneViewAddress, type);
    httpData = getRequestWithUrlAndHeader(urlString, sessionID);
    if(!httpData)
        return 1;
    // Process the raw http data and free the allocated memory
    root = json_loads(httpData, 0, &error);
    char *json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
    if (json_text) {
        printf("%s\n", json_text);
        free(json_text);
    }
    json_decref(root);
    free(httpData);
    return 0;
}


int ovMsgBusListen(char *argument[], char *path)
{
    // Count the size of arguments passed
    int count = 0;
    while (argument[++count] !=NULL);
    if (count < 5) { // No Message Bus
        return -1;
    }
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *messageBus = argument[4]; // Connect to either the state or the metric bus
    if (stringMatch(messageBus, "STATE")) {
        messageBus = "scmb";
    } else if (stringMatch(messageBus, "METRIC")) {
        messageBus = "msmb";
    } else {
        printf("Incorrect Message bus to connect to\n");
        return -1;
    }
    char *routingKey = argument[5]; // RabbitMQ Routing Key (Search params)
    if (!routingKey) {
        printf("No routing key specified e.g. scmb.#\n");
        return  -1;
    }
    int output = OVSTDOUT;
    // If no parameter passed then default to cmd line
    if (count < 7) {
        output = OVSTDOUT;
    } else {
        if (stringMatch(argument[6], "STDOUT")) {
            output = OVSTDOUT;
        } else if (stringMatch(argument[6], "FILE")) {
            output = OVFILE;
        } else if (stringMatch(argument[6], "HTTP")) {
            if (count < 7) {
                printf("\nHTTP Url required\n");
                return -1;
            }
            output = OVHTTP;
        } else if (stringMatch(argument[6], "INFLUXDB")) {
            if (count < 8) {
                printf("\nInfluxDB DB name required\n");
                return -1;
            }
            if (count < 7) {
                printf("\nInfluxDB IP required\n");
                return -1;
            }
            output = OVINFLUXDB;
        }
    }
    
    
    int port = 5671; // Static port allocation
    amqp_bytes_t queuename;
    amqp_socket_t *socket;
    amqp_connection_state_t conn;
    
    // JSON Processing from MessageBus
    json_t *root; // Contains all of the json data once processed by jansson
    json_error_t error; // Used as a passback for error data during json processing

    // Character arrays for paths to certificates
    char cacert[250], cert[250], key[250], home[150];
    
    strcpy(home, getenv("HOME")); // copy in the $HOME env into the string
    sprintf(cacert, "%s%s_cacert", home, path); // Create the path to the root certificate
    sprintf(cert, "%s%s_pem", home, path); // Create the path to the certificate for RabbitMQ
    sprintf(key, "%s%s_key", home, path); // Create the path to the key

    // Create connection structure
    conn = amqp_new_connection();
    // Assign a TCP socket to the connection
    socket = amqp_ssl_socket_new(conn);
    if (!socket) {
        printf("Connection failed");
        return -1;
    }

    if (amqp_ssl_socket_set_cacert(socket, cacert)) {
        printf("\nError with Certificate Authority Certificate\n");
        return -1;
    }
    if (amqp_ssl_socket_set_key(socket, cert, key)) {
        printf("\nError with Key or Certificate\n");
        return -1;
    }
    if (amqp_socket_open(socket, oneViewAddress, port)) {
        printf("\nConnection to RabbitMQ Server has failed, check IP or Port\n");
        return -1;
    }

    // Wrap AMQP Functions with amqpGetStatus (), which will automatically check if there has been any errors whilst runngin a function
    // Login to Server using existing connection AMQP_SASL_METHOD_EXTERNAL will use the certificates noted above.
    amqpGetStatus(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_EXTERNAL, "guest", "guest"));

    amqp_channel_open(conn, 1);

    amqpGetStatus(amqp_get_rpc_reply(conn));

    // Create a queue localhost to handle the incoming messages
    {
        amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
        amqp_get_rpc_reply(conn);
        queuename = amqp_bytes_malloc_dup(r->queue);
        if (queuename.bytes == NULL) {
            fprintf(stderr, "Out of memory while copying queue name");
            return 1;
        }
    }

    // Bind our new queue to the exchange and use a routing Key to (possibly) reduce the amount of messages
    amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(messageBus), amqp_cstring_bytes(routingKey), amqp_empty_table);

    amqpGetStatus(amqp_get_rpc_reply(conn));
    // Start consuming messages on the queue
    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
    
    amqpGetStatus(amqp_get_rpc_reply(conn));

   // Process Messages
    {
        while (1) {
            amqp_rpc_reply_t res;
            amqp_envelope_t envelope;
            char *body;
            char filename[FILENAME_MAX];
            amqp_maybe_release_buffers(conn);
        
            res = amqp_consume_message(conn, &envelope, NULL, 0);
        
            if (AMQP_RESPONSE_NORMAL != res.reply_type) {
                break;
            }

            body = strndup(envelope.message.body.bytes, envelope.message.body.len);
            
            switch (output) {
                case OVSTDOUT:
                    // Print all of the Messages from the Bus to the CLI
                    printf("Delivery %u, exchange %.*s routingkey %.*s\n",
                           (unsigned) envelope.delivery_tag,
                           (int) envelope.exchange.len, (char *) envelope.exchange.bytes,
                           (int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);
                    
                    if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
                        printf("Content-type: %.*s\n",
                               (int) envelope.message.properties.content_type.len,
                               (char *) envelope.message.properties.content_type.bytes);
                    }
                    printf("----\n");
                    
                    // Parse the text as JSON and then have it INDENTED (readable)
                    root = json_loads(body, 0, &error);
                    char *json_text = json_dumps(root, JSON_INDENT(4)); //4 is close to a tab
                    if (!json_text) {
                        // There has been an error with the libjansson library parsing the text
                        printf("\n%s\n", body);
                        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
                        printf("String Length: %zu, versus actual length: %lu \n", envelope.message.body.len, strlen(body));
                    } else {
                        printf("%s\n", json_text);
                        free(json_text); // Free only if it's been allocated otherwise can lead to an error
                    }
                    // Tidy up
                    json_decref(root);
                    break;
                case OVFILE:
                    // Save every JSON Message to a numbered file
                    sprintf(filename, "~/%u.json", (unsigned) envelope.delivery_tag);
                    writeDataToFile(body, filename);
                    break;
                case OVHTTP:
                {
                    // Post messages from the Message Bus to another web server
                    char urlString[strlen(argument[7])+2];
                    sprintf(urlString, "%s/%u", argument[7], (unsigned) envelope.delivery_tag);
                    postRequestWithUrlAndData(argument[7], body);
                    break;
                }
                case OVINFLUXDB:
                    postToServer(argument[7], argument[8],body);
                    break;
                default:
                    break;
            }
            if (body) {
                // Tidy up cloned strings
                free(body);
            }
            amqp_destroy_envelope(&envelope);
        }
    }
    return 0;
}

