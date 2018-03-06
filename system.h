#ifndef _SYSTEM_H
#define _SYSTEM_H   1

#include <avr/io.h>

#define F_CPU 32000000UL

void setUp32MhzInternalOsc(void)
{
    OSC_CTRL |= OSC_RC32MEN_bm; //Setup 32Mhz crystal
     
    while(!(OSC_STATUS & OSC_RC32MRDY_bm));
     
    CCP = CCP_IOREG_gc; //Trigger protection mechanism
    CLK_CTRL = CLK_SCLKSEL_RC32M_gc; //Enable internal  32Mhz crystal
}



#endif