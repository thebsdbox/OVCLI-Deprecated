//
//  dcHttp.h
//  OVCLI
//
//  Created by Daniel Finneran on 10/11/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#ifndef dcHttp_h
#define dcHttp_h

#include <stdio.h>

#endif /* dcHttp_h */

void setHttpAuth(char* authString);
void appendHttpHeader(char *header);
void setHttpData(char* dataString);
void SetHttpMethod(int method);
char *httpFunction(const char *url);
void PrintHttpAuth();
void createHeader(char *key, char *data);

#define DCHTTPPOST     0 // POST Operation
#define DCHTTPPUT      1 // PUT Operation
#define DCHTTPGET      2 // GET Operation