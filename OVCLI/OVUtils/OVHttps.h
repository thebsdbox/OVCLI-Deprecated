//
//  OVHttps.h
//  OVCLI
//
//  Created by Daniel Finneran on 22/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#ifndef __OVCLI__OVHttps__
#define __OVCLI__OVHttps__

#include <stdio.h>

#include <curl/curl.h>

#endif /* defined(__OVCLI__OVHttps__) */


char *postRequestWithUrl(const char *url);
char *postRequestWithUrlAndData(const char *url, const char *postData);
char *postRequestWithUrlAndDataAndHeader(const char *url, const char *postData, const char *header);
char *httpRequest(const char *url, const char *postData, const char *header, const char *putData);
char *getRequestWithUrl(const char *url);
char *getRequestWithUrlAndHeader(const char *url, const char *header);
char *putRequestWithURLAndDataAndHeader(const char *url, const char *putData, const char *header);