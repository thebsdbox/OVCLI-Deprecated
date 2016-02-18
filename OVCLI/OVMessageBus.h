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

#define OVSTDOUT   0 // Write out to command line
#define OVFILE     1 // Write out to .json fies
#define OVHTTP     2 // Proxy the messages from HP OneView to another server
#define OVINFLUXDB 3 // Proxy the messages from HP OneView to an InfluxDB server

int ovMsgBusCertGenerate(char *sessionID, char *argument[]);
int ovMsgBusCertDownload(char *sessionID, char *argument[], char *path);

int ovMsgBusListen(char *argument[], char *path);
int ovMetricMsgBusGetSettings(char *sessionID, char *argument[]);
void ovMessageBusHelp();