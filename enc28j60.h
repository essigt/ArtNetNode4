#ifndef _ENC28J60_H
#define _ENC28J60_H  1


#include "enc28j60_defs.h"

#include <avr/io.h>

uint8_t buffer[MAX_FRAMELEN];

extern uint8_t setupEthernet(void);

extern uint16_t packetReceive(void);

#endif