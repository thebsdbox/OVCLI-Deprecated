//
//  dcHttp.c
//  OVCLI
//
//  Created by Daniel Finneran on 10/11/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#include "dcHttp.h"

// CURL Header
#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

#include "OVOutput.h"

char *httpsAuth; // String set as User:Pass
char *httpData; // Data to send to web service
int httpMethod; // Method used to send Data to WebServer

struct curl_slist *headers = NULL;

#define BUFFER_SIZE  (1024 * 1024)  /* 1024 KB */

struct write_result
{
    char *data;
    int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct write_result *result = (struct write_result *)stream;
    
    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }
    
    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;
    
    return size * nmemb;
}

void setHttpAuth(char* authString)
{
    httpsAuth = authString;
}

void appendHttpHeader(char *header)
{
    // Append Headers to header list
    headers = curl_slist_append(headers, (const char*) header);
}

void createHeader(char *key, char *data)
{
    char headerData[strlen(key)+strlen(data)];
    //char headerData[56]; //6 for auth: and 48 (33 < 1.20 version) for sessionID
    strcpy(headerData, key);
    strcat(headerData, data);
    appendHttpHeader(headerData);
}

void setHttpData(char* dataString)
{
    httpData = dataString;
}

void SetHttpMethod(int method)
{
    if (method > 3) {
        printf("\nError: Unknown HTTP Method\n");
        exit(-1);
    }
    httpMethod = method;
}

// httpRequest function
char *httpFunction(const char *url)
{
    if (!url) {
        printf("\n No URL specified\n");
        return "";
    }
    
    CURL *curl = NULL;
    CURLcode status;
    char *data = NULL;
    long code;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl)
        goto error;
    
    data = malloc(BUFFER_SIZE);
    if(!data)
        goto error;
    
    struct write_result write_result = {
        .data = data,
        .pos = 0
    };
    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); /* This is due to self signed Certs */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* HP OneView needs a Content-Type setting*/
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); // Give the connection process a 10 second timeout.
    if (httpMethod < 2) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, httpData);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
    
    switch (httpMethod) {
        case DCHTTPPOST:
            break;
        case DCHTTPPUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case DCHTTPGET:
            break;
        default:
            break;
    }
    if (httpsAuth) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, httpsAuth);
    }
    
    status = curl_easy_perform(curl);
    if(status != 0)
    {
        fprintf(stderr, "[ERROR] unable to request data from %s:\n", url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));
        goto error;
    }
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    
    switch (code) {
        case 200:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_GREEN"[DEBUG] POST Success\n"ANSI_COLOR_RESET);
            break;
        case 201:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_GREEN"[DEBUG] POST Submitted\n"ANSI_COLOR_RESET);
            break;
        case 202:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_GREEN"[DEBUG] POST Accepted\n"ANSI_COLOR_RESET);
            break;
        case 400:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_RED "[DEBUG]" ANSI_COLOR_RESET " Malformed request something in the JSON is broken");
            if (data)
                printf("%s", data);
            break;
        case 401:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_RED "[DEBUG]" ANSI_COLOR_RESET " POST Failure (try logging back in) \n");
            break;
        case 409:
            if (getenv("OV_DEBUG"))
                printf(ANSI_COLOR_RED "[DEBUG]" ANSI_COLOR_RESET " Error 409 - Conflict error (request may have succeded)\n");
            break;
        default:
            break;
    }
    
    if(code > 500)
    {
        if (getenv("OV_DEBUG"))
            fprintf(stderr, ANSI_COLOR_RED"[ERROR]" ANSI_COLOR_RESET" server responded with code %ld\n", code);
        goto error;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    // Set headers to NULL so that they can be reallocated by headers_append()
    headers = NULL;
    curl_global_cleanup();
    
    /* zero-terminate the result */
    data[write_result.pos] = '\0';
    
    return data;
    
error:
    if(data)
        free(data);
    if(curl)
        curl_easy_cleanup(curl);
    if(headers) {
        curl_slist_free_all(headers);
        headers = NULL;
    }
    curl_global_cleanup();
    return NULL;
}

void PrintHttpAuth()
{
    if (httpsAuth) {
        printf ("%s\n", httpsAuth);
    }
}