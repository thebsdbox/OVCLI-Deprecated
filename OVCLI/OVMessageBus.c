//
//  OVMessageBus.c
//  OVCLI
//
//  Created by Daniel Finneran on 15/09/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include <ctype.h>

#include "OVUtils.h"
#include "OVMessageBus.h"
#include "OVHttps.h"
#include "jansson.h"

// RabbitMQ
#include "amqp_ssl_socket.h"
#include "amqp_framing.h"

int ovMessageBusGenerate(char *sessionID, char *argument[])
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

int ovMessageBusCertificates (char *sessionID, char *argument[], char *path)
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


///// THE BELOW FUNCTIONS WILL BE REMOVED


static void dump_row(long count, int numinrow, int *chs)
{
    int i;
    
    printf("%08lX:", count - numinrow);
    
    if (numinrow > 0) {
        for (i = 0; i < numinrow; i++) {
            if (i == 8) {
                printf(" :");
            }
            printf(" %02X", chs[i]);
        }
        for (i = numinrow; i < 16; i++) {
            if (i == 8) {
                printf(" :");
            }
            printf("   ");
        }
        printf("  ");
        for (i = 0; i < numinrow; i++) {
            if (isprint(chs[i])) {
                printf("%c", chs[i]);
            } else {
                printf(".");
            }
        }
    }
    printf("\n");
}

static int rows_eq(int *a, int *b)
{
    int i;
    
    for (i=0; i<16; i++)
        if (a[i] != b[i]) {
            return 0;
        }
    
    return 1;
}

void amqp_dump(void const *buffer, size_t len)
{
    unsigned char *buf = (unsigned char *) buffer;
    long count = 0;
    int numinrow = 0;
    int chs[16];
    int oldchs[16] = {0};
    int showed_dots = 0;
    size_t i;
    
    for (i = 0; i < len; i++) {
        int ch = buf[i];
        
        if (numinrow == 16) {
            int i;
            
            if (rows_eq(oldchs, chs)) {
                if (!showed_dots) {
                    showed_dots = 1;
                    printf("          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
                }
            } else {
                showed_dots = 0;
                dump_row(count, numinrow, chs);
            }
            
            for (i=0; i<16; i++) {
                oldchs[i] = chs[i];
            }
            
            numinrow = 0;
        }
        
        count++;
        chs[numinrow++] = ch;
    }
    
    dump_row(count, numinrow, chs);
    
    if (numinrow != 0) {
        printf("%08lX:\n", count);
    }
}


///// THE ABOVE FUNCTIONS WILL BE REMOVED


int ovMessageBusListen(char *argument[], char *path)
{
    char const *hostname;
    int port, status;
    amqp_bytes_t queuename;
    amqp_socket_t *socket;
    amqp_connection_state_t conn;

    char cacert[150];
    char cert[150];
    char key[150];
    char home[150];
    strcpy(home, getenv("HOME")); // copy in the $HOME env into the string

    sprintf(cacert, "%s%s_cacert", home, path);
    sprintf(cert, "%s%s_pem", home, path);
    sprintf(key, "%s%s_key", home, path);

    hostname = "192.168.0.91";
    port = 5671;

    conn = amqp_new_connection();

    socket = amqp_ssl_socket_new(conn);
    if (!socket) {
        printf("Connection failed");
        return -1;
    }

    status = amqp_ssl_socket_set_cacert(socket, cacert);
    status = amqp_ssl_socket_set_key(socket, cert, key);
    status = amqp_socket_open(socket, hostname, port);

    amqp_rpc_reply_t x = amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_EXTERNAL, "guest", "guest");

    fprintf(stderr, "%s: %s\n", "Logging in", amqp_error_string2(x.library_error));

    amqp_channel_open(conn, 1);

    x =amqp_get_rpc_reply(conn);

    {
        amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1,
                                                        amqp_empty_table);
        amqp_get_rpc_reply(conn);
        queuename = amqp_bytes_malloc_dup(r->queue);
        if (queuename.bytes == NULL) {
            fprintf(stderr, "Out of memory while copying queue name");
            return 1;
        }
    }

    amqp_queue_bind(conn, 1,
                    queuename,
                    amqp_cstring_bytes("scmb"),
                    amqp_cstring_bytes("scmb.#"),
                    amqp_empty_table);

        x =amqp_get_rpc_reply(conn);

    switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
            amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
            fprintf(stderr, "%s: server connection error %d, message: %.*s\n",
                    "",
                    m->reply_code,
                    (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
            amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
            fprintf(stderr, "%s: server channel error %d, message: %.*s\n",
                    "",
                    m->reply_code,
                    (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
        }
        default:
            fprintf(stderr, "%s: unknown server error, method id 0x%08X\n", "", x.reply.id);
            break;
    }

    fprintf(stderr, "%s: %s\n", "Logging in", amqp_error_string2(x.library_error));



    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
    x= amqp_get_rpc_reply(conn);


    {
        while (1) {
            amqp_rpc_reply_t res;
            amqp_envelope_t envelope;
        
            amqp_maybe_release_buffers(conn);
        
            res = amqp_consume_message(conn, &envelope, NULL, 0);
        
            if (AMQP_RESPONSE_NORMAL != res.reply_type) {
                break;
            }
        
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
        
            amqp_dump(envelope.message.body.bytes, envelope.message.body.len);
        
            amqp_destroy_envelope(&envelope);
        }
    }
    return 0;
}

