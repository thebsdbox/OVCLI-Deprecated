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

int ovMessageBusCertificates (char *sessionID, char *argument[], char *path);
int ovMessageBusGenerate(char *sessionID, char *argument[]);
int ovMessageBusListen(char *argument[], char *path);