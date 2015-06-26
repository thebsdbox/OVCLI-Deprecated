//
//  OVOutput.h
//  OVCLI
//
//  Created by Daniel Finneran on 03/06/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#ifndef OVCLI_OVOutput_h
#define OVCLI_OVOutput_h

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Used as part of the function
#define RED     0
#define GREEN   1
#define YELLOW  2
#define BLUE    3
#define MAGENTA 4
#define CYAN    5

#endif

int printMessage(int labelColour, char *level, char *text);