#include "dmx.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>




uint8_t dmxBuffer[DMX_NUM_UNIVERSES][DMX_UNIVERSE_SIZE];

uint8_t* getUniverseBuffer(uint8_t universe) {
    return dmxBuffer[universe];
}



enum DMX_STATE {
    DMX_IDLE,
    DMX_BREAK,
    DMX_START,
    DMX_RUN
};


enum DMX_STATE dmx0state = DMX_IDLE;
enum DMX_STATE dmx1state = DMX_IDLE;
enum DMX_STATE dmx2state = DMX_IDLE;
enum DMX_STATE dmx3state = DMX_IDLE;
uint16_t dmx0ch = 0;
uint16_t dmx1ch = 0;
uint16_t dmx2ch = 0;
uint16_t dmx3ch = 0;


#define DMX_BAUD_BSCALE          0   // vorher 0b1001 -> -1
#define DMX_BAUD_RESET_BSEL     15   // 125k BAUD @32Mhz, used for RESET
#define DMX_BAUD_NORMAL_BSEL     7   // 250k BAUD @32Mhz

#define UART0 USARTD0

void setupDMXPort(USART_t* uart) {
    uart->BAUDCTRLB = DMX_BAUD_BSCALE << 4;
    uart->BAUDCTRLA = DMX_BAUD_RESET_BSEL;

    uart->CTRLA = USART_TXCINTLVL_gm; // TX Highest INT Level    
    uart->CTRLC = USART_SBMODE_bm | USART_CHSIZE_8BIT_gc; //2 StopBits, 8 DataBits

    uart->CTRLB = USART_TXEN_bm; //Enable TX
}

void setupDMX(void) {
    //DMX 1
    PORTC.DIRSET = PIN3_bm; // Pin 3 -> UART 0 TX
    PORTC.PIN3CTRL = PORT_INVEN_bm;    
    setupDMXPort(&USARTC0);

    //DMX 2
    PORTC.DIRSET = PIN7_bm; // Pin 3 -> UART 0 TX
    PORTC.PIN7CTRL = PORT_INVEN_bm;    
    setupDMXPort(&USARTC1);


    //DMX 3
    PORTD.DIRSET = PIN3_bm; // Pin 3 -> UART 0 TX
    PORTD.PIN3CTRL = PORT_INVEN_bm;    
    setupDMXPort(&USARTD0);

    //DMX 4
    PORTE.DIRSET = PIN3_bm; // Pin 3 -> UART 0 TX
    PORTE.PIN3CTRL = PORT_INVEN_bm;    
    setupDMXPort(&USARTE0);
}


void startDMX_TX(void) {    
    if(dmx0state == DMX_IDLE) {
        //Set Baud
        UART0.BAUDCTRLA = DMX_BAUD_RESET_BSEL;
        UART0.BAUDCTRLB = DMX_BAUD_BSCALE << 4;

        //Send Break Byte
        UART0.DATA = 0x00;

        dmx0state = DMX_START;
    }
}

ISR(USARTD0_TXC_vect) //Transimission complete
{

    switch(dmx0state) {
        case DMX_START:
            //Change Baudrate
            UART0.BAUDCTRLA = DMX_BAUD_NORMAL_BSEL;
            UART0.BAUDCTRLB = DMX_BAUD_BSCALE << 4;

            //SEND START Byte
            UART0.DATA = 0x00;


            dmx0ch = 0;
            dmx0state = DMX_RUN;

        break;
        case DMX_RUN:
            UART0.DATA = dmxBuffer[0][dmx0ch++];            
            
            if(dmx0ch >= 512) {
                dmx0state = DMX_IDLE;
            }
        break;

    }
}
