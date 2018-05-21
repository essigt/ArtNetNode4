#include "timer.h"

#include "artnet.h"
#include "dmx.h"

#include <avr/io.h>
#include <avr/interrupt.h>


//Different counters
volatile uint16_t timer01hzCounter = 0;
volatile uint8_t timer022HZCounter = 0;

volatile uint32_t millisCounter = 0;


void setupTimer0(void) {

    //################# Timer0 - 1Khz
    TCC0.CTRLA = TC_CLKSEL_DIV1024_gc; // Presacler 1024
    TCC0.CTRLB = 0x00; // select Modus: Normal
    TCC0.PER = 32; // Zähler Top-Wert 1000 Hz 
    TCC0.CNT = 0x00; // Zähler zurücksetzen
    TCC0.INTCTRLA = 0b00000011; // Interrupt Highlevel
}


uint32_t millis(void) {
    return millisCounter;
}

ISR(TCC0_OVF_vect) 
{
    timer01hzCounter++; 
    millisCounter++;

    if(timer01hzCounter >= 1000) {
        timer01hzCounter = 0;
        artnet.doTimerStuff1Hz(); //Art-Net
    }

    timer022HZCounter++;
    if(timer022HZCounter >= 23) {
        timer022HZCounter = 0;
        startDMX_TX(); //Reicht? TODO: Timer, 44mal/sec
    }
    
}