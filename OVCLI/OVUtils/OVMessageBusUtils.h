//
//  OVMessageBusUtils.h
//  OVCLI
//
//  Created by Daniel Finneran on 23/09/2015.
//  Copyright © 2015 Daniel Finneran. All rights reserved.
//

#ifndef OVMessageBusUtils_h
#define OVMessageBusUtils_h

#include <stdio.h>
#include "amqp.h"
#endif /* OVMessageBusUtils_h */


int amqpGetStatus(amqp_rpc_reply_t x);

void amqp_dump(void const *buffer, size_t len);
