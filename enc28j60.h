#ifndef _ENC28J60_H
#define _ENC28J60_H  1

#include <avr/io.h>


extern uint8_t setupEthernet(void);

extern uint16_t packetReceive(void);
extern void packageLoop(uint16_t len);

#endif