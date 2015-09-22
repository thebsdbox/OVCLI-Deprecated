//
//  OVOutput.c
//  OVCLI
//
//  Created by Daniel Finneran on 19/06/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//
#include "OVOutput.h"
#include <stdio.h>

int printMessage(int labelColour, char *level, char *text)
{
    if (debug)
    {
        switch (labelColour) {
            case RED:
                printf("["ANSI_COLOR_RED"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            case GREEN:
                printf("["ANSI_COLOR_GREEN"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            case YELLOW:
                printf("["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            case BLUE:
                printf("["ANSI_COLOR_BLUE"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            case MAGENTA:
                printf("["ANSI_COLOR_MAGENTA"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            case CYAN:
                printf("["ANSI_COLOR_CYAN"%s"ANSI_COLOR_RESET"] %s\n", level, text);
                break;
            default:
                break;
        }
    }
    //printf(ANSI_COLOR_GREEN"[DEBUG] POST Success\n"ANSI_COLOR_RESET);
    return 0;
    
}