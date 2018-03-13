#ifndef _ARTNET_H
#define _ARTNET_H   1

#include <avr/io.h>

// Port to listen on
#define UDP_PORT_ARTNET       6454  /* (0x1936) */
// Port to reply on
#define UDP_PORT_ARTNET_REPLY (UDP_PORT_ARTNET + 1)


extern uint8_t getUniverseFPS(uint8_t universe);

extern void universeReceivedTMP(uint8_t universe);

extern void doTimerStuff1Hz(void);

#endif