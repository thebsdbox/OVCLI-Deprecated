//
//  libinfluxdb.h
//  OVCLI
//
//  Created by Daniel Finneran on 13/10/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#ifndef libinfluxdb_h
#define libinfluxdb_h

#include <stdio.h>

#endif /* libinfluxdb_h */

// Struct to hold all of the Server Details
typedef struct ovServerHardware {
    long avgPower;
    long peakPower;
    long powerCap;
    long temp;
    long cpuUse;
    long cpu;
    char serverURI[58]; // Length of serverURI
} ovServerHardware;

int postToServer(char *address, char *database, char *data);
