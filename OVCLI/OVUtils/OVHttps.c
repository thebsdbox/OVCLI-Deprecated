 //
//  OVHttps.c
//  OVCLI
//
//  Created by Daniel Finneran on 22/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVHttps.h"
#include <stdlib.h>
#include <string.h>

#include "OVOutput.h"



#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

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

// Post Requests (wrappers around HttpRequest)
char *postRequestWithUrl(const char *url)
{
    return httpRequest(url, NULL, NULL, NULL);
}

char *postRequestWithUrlAndData(const char *url, const char *postData)
{
    return httpRequest(url, postData, NULL, NULL);
}

char *postRequestWithUrlAndDataAndHeader(const char *url, const char *postData, const char *header)
{
    return httpRequest(url, postData, header, NULL);
}

// Put Request (wrapper around HttpRequest)
char *putRequestWithURLAndDataAndHeader(const char *url, const char *putData, const char *header)
{
    return httpRequest(url, NULL, header, putData);
}


// Get Request (wrapper around post requests, no data sent)
char *getRequestWithUrl(const char *url)
{
    return postRequestWithUrl(url);
}

char *getRequestWithUrlAndHeader(const char *url, const char *header)
{
    return postRequestWithUrlAndDataAndHeader(url, NULL, header);
}


// httpRequest function
char *httpRequest(const char *url, const char *postData, const char *header, const char *putData)
{
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
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
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* HP OneView needs a Content-Type setting*/

    if (header)
    {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "X-API-Version: 200");
        // Append the Session ID to the headers
        char authHeader[56]; //6 for auth: and 48 (33 < 1.20 version) for sessionID
        strcpy(authHeader, "Auth: ");
        strcat(authHeader, header);
        headers = curl_slist_append(headers, authHeader);
    }
    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); /* This is due to self signed Certs */
    if (postData) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    }
    
    if (putData) {
        //specify the request (PUT in our case), the post as normal
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, putData);
        headers = NULL;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); // Give the connection process a 10 second timeout.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
    //curl_easy_setopt(curl, CURLOPT_USERPWD, "admin:admin");
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
                printf(ANSI_COLOR_RED "[DEBUG]" ANSI_COLOR_RESET " Error 409 \n");
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
    curl_global_cleanup();
    
    /* zero-terminate the result */
    data[write_result.pos] = '\0';
    
    return data;
    
error:
    if(data)
        free(data);
    if(curl)
        curl_easy_cleanup(curl);
    if(headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();
    return NULL;
}



