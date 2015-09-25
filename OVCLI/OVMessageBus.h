//
//  OVMessageBus.h
//  OVCLI
//
//  Created by Daniel Finneran on 15/09/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#ifndef __OVCLI__OVMessageBus__
#define __OVCLI__OVMessageBus__

#include <stdio.h>

#endif /* defined(__OVCLI__OVMessageBus__) */

int ovMsgBusCertGenerate(char *sessionID, char *argument[]);
int ovMsgBusCertDownload(char *sessionID, char *argument[], char *path);

int ovMsgBusListen(char *argument[], char *path);
