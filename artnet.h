#ifndef _ARTNET_H
#define _ARTNET_H   1

#include <avr/io.h>


extern uint8_t getUniverseFPS(uint8_t universe);

extern void universeReceivedTMP(uint8_t universe);

extern void doTimerStuff1Hz(void);

#endif