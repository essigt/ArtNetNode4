#ifndef _DMX_H
#define _DMX_H   1

#include <avr/io.h>

#define DMX_NUM_UNIVERSES    4
#define DMX_UNIVERSE_SIZE  512

extern uint8_t dmxBuffer[DMX_NUM_UNIVERSES][DMX_UNIVERSE_SIZE];


extern uint8_t* getUniverseBuffer(uint8_t universe);


extern void setupDMX(void);
extern void startDMX_TX(void);
//extern void startDMX_TX(void);


#endif