//
//  OVMessageBusUtils.c
//  OVCLI
//
//  Created by Daniel Finneran on 23/09/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#include "OVMessageBusUtils.h"
#include "OVOutput.h"

#include <ctype.h>

// DEBUG OUTPUT from RabbitMQ

///// THE BELOW FUNCTIONS WILL BE REMOVED


static void dump_row(long count, int numinrow, int *chs)
{
    int i;
    
    printf("%08lX:", count - numinrow);
    
    if (numinrow > 0) {
        for (i = 0; i < numinrow; i++) {
            if (i == 8) {
                printf(" :");
            }
            printf(" %02X", chs[i]);
        }
        for (i = numinrow; i < 16; i++) {
            if (i == 8) {
                printf(" :");
            }
            printf("   ");
        }
        printf("  ");
        for (i = 0; i < numinrow; i++) {
            if (isprint(chs[i])) {
                printf("%c", chs[i]);
            } else {
                printf(".");
            }
        }
    }
    printf("\n");
}

static int rows_eq(int *a, int *b)
{
    int i;
    
    for (i=0; i<16; i++)
        if (a[i] != b[i]) {
            return 0;
        }
    
    return 1;
}

void amqp_dump(void const *buffer, size_t len)
{
    unsigned char *buf = (unsigned char *) buffer;
    long count = 0;
    int numinrow = 0;
    int chs[16];
    int oldchs[16] = {0};
    int showed_dots = 0;
    size_t i;
    
    for (i = 0; i < len; i++) {
        int ch = buf[i];
        
        if (numinrow == 16) {
            int i;
            
            if (rows_eq(oldchs, chs)) {
                if (!showed_dots) {
                    showed_dots = 1;
                    printf("          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
                }
            } else {
                showed_dots = 0;
                dump_row(count, numinrow, chs);
            }
            
            for (i=0; i<16; i++) {
                oldchs[i] = chs[i];
            }
            
            numinrow = 0;
        }
        
        count++;
        chs[numinrow++] = ch;
    }
    
    dump_row(count, numinrow, chs);
    
    if (numinrow != 0) {
        printf("%08lX:\n", count);
    }
}


///// THE ABOVE FUNCTIONS WILL BE REMOVED

int amqpGetStatus(amqp_rpc_reply_t reply)
{
    if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
        return 0;
    } else{
        switch (reply.reply_type) {
            case AMQP_RESPONSE_NONE:
                printMessage(YELLOW, "DEBUG", "No Response from RPC");
                break;
            case AMQP_RESPONSE_LIBRARY_EXCEPTION:
                printMessage(RED, "ERROR", "Possible SSL/TLS error during certificate negotiations, check home directory");
                printf("Library Error, possible TLS/SSL Error :\n %s\n", amqp_error_string2(reply.library_error));
                break;
            case AMQP_RESPONSE_SERVER_EXCEPTION:
                printMessage(RED, "ERROR", "Issue could be authentication or issue with the MQ Channel");
                switch (reply.reply.id) {
                    case AMQP_CONNECTION_CLOSE_METHOD: {
                        amqp_connection_close_t *m = (amqp_connection_close_t *) reply.reply.decoded;
                        fprintf(stderr, "Server connection error %d, message: %.*s\n",
                                m->reply_code,
                                (int) m->reply_text.len, (char *) m->reply_text.bytes);
                        break;
                    }
                    case AMQP_CHANNEL_CLOSE_METHOD: {
                        amqp_channel_close_t *m = (amqp_channel_close_t *) reply.reply.decoded;
                        fprintf(stderr, "Server channel error %d, message: %.*s\n",
                                m->reply_code,
                                (int) m->reply_text.len, (char *) m->reply_text.bytes);
                        break;
                    }
                    default:
                        fprintf(stderr, "Unknown server error, method id 0x%08X\n", reply.reply.id);
                        break;
                }
                break;

            default:
            
                break;
        }
    }

    return -1;
}