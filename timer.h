#ifndef _TIMER_H
#define _TIMER_H   1

#include <avr/io.h>


extern void setupTimer0(void);

extern uint32_t millis(void);

#endif