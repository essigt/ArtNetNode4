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

typedef struct DMX_PORT {
    PORT_t*         PORT;
    uint8_t         PIN;
    register8_t*    PIN_CTRL;
    USART_t*        USART;
    uint8_t         UNIVERSE;
    uint8_t         OUTPUT_PORT;
    enum DMX_STATE  TX_STATE;
    uint16_t        TX_POSITION;

} DMX_t;


volatile DMX_t dmxPorts[DMX_NUM_UNIVERSES];


#define DMX_BAUD_BSCALE          0   // vorher 0b1001 -> -1
#define DMX_BAUD_RESET_BSEL     15   // 125k BAUD @32Mhz, used for RESET
#define DMX_BAUD_NORMAL_BSEL     7   // 250k BAUD @32Mhz

void setupDMXPort(DMX_t* PORT) {
    PORT->PORT->DIRSET = 1 << PORT->PIN; // Pin 3 -> UART 0 TX
    *PORT->PIN_CTRL = PORT_INVEN_bm;    

    PORT->USART->BAUDCTRLB = DMX_BAUD_BSCALE << 4;
    PORT->USART->BAUDCTRLA = DMX_BAUD_RESET_BSEL;

    PORT->USART->CTRLA = USART_TXCINTLVL_gm; // TX Highest INT Level    
    PORT->USART->CTRLC = USART_SBMODE_bm | USART_CHSIZE_8BIT_gc; //2 StopBits, 8 DataBits

    PORT->USART->CTRLB = USART_TXEN_bm; //Enable TX
}

void setupDMX(void) {
    dmxPorts[0].PORT = &PORTC;
    dmxPorts[0].PIN = 3;
    dmxPorts[0].PIN_CTRL = &PORTC.PIN3CTRL;
    dmxPorts[0].USART   = &USARTC0;
    dmxPorts[0].UNIVERSE = 0;
    dmxPorts[0].OUTPUT_PORT = 1;
    dmxPorts[0].TX_STATE = DMX_IDLE;

    dmxPorts[1].PORT = &PORTC;
    dmxPorts[1].PIN = 7;
    dmxPorts[1].PIN_CTRL = &PORTC.PIN7CTRL;
    dmxPorts[1].USART   = &USARTC1;
    dmxPorts[1].UNIVERSE = 1;
    dmxPorts[1].OUTPUT_PORT = 2;
    dmxPorts[1].TX_STATE = DMX_IDLE;

    dmxPorts[2].PORT = &PORTD;
    dmxPorts[2].PIN = 3;
    dmxPorts[2].PIN_CTRL = &PORTD.PIN3CTRL;
    dmxPorts[2].USART   = &USARTD0;
    dmxPorts[2].UNIVERSE = 2;
    dmxPorts[2].OUTPUT_PORT = 3;
    dmxPorts[2].TX_STATE = DMX_IDLE;

    dmxPorts[3].PORT = &PORTE;
    dmxPorts[3].PIN = 3;
    dmxPorts[3].PIN_CTRL = &PORTE.PIN3CTRL;
    dmxPorts[3].USART   = &USARTE0;
    dmxPorts[3].UNIVERSE = 3;
    dmxPorts[3].OUTPUT_PORT = 4;
    dmxPorts[3].TX_STATE = DMX_IDLE;


    for(uint8_t i=0; i < DMX_NUM_UNIVERSES; i++) {
        setupDMXPort(&dmxPorts[i]);
    }
}


void startDMX_TX(void) {
    for(uint8_t i=0; i < DMX_NUM_UNIVERSES; i++) {
        if(dmxPorts[i].TX_STATE == DMX_IDLE) {
            //Set Baud
            dmxPorts[i].USART->BAUDCTRLA = DMX_BAUD_RESET_BSEL;
            dmxPorts[i].USART->BAUDCTRLB = DMX_BAUD_BSCALE << 4;

            //Send Break Byte
            dmxPorts[i].USART->DATA = 0x00;

            dmxPorts[i].TX_STATE = DMX_START;
        }
    }
}

/**
 * Handle DMX Tx Interrupt
 */
void txISR(DMX_t* PORT) {
    switch(PORT->TX_STATE) {
        case DMX_START:
            //Change Baudrate
            PORT->USART->BAUDCTRLA = DMX_BAUD_NORMAL_BSEL;
            PORT->USART->BAUDCTRLB = DMX_BAUD_BSCALE << 4;

            //SEND START Byte
            PORT->USART->DATA = 0x00;

            PORT->TX_POSITION = 0;
            PORT->TX_STATE = DMX_RUN;
        break;
        case DMX_RUN:
            PORT->USART->DATA = dmxBuffer[PORT->UNIVERSE][PORT->TX_POSITION++];
            
            if(PORT->TX_POSITION >= 512) {
                PORT->TX_STATE = DMX_IDLE;
            }
        break;

    }
}

ISR(USARTC0_TXC_vect) //Transimission complete
{
    txISR(&dmxPorts[0]);
}

ISR(USARTC1_TXC_vect) //Transimission complete
{
    txISR(&dmxPorts[1]);
}

ISR(USARTD0_TXC_vect) //Transimission complete
{
    txISR(&dmxPorts[2]);
}

ISR(USARTE0_TXC_vect) //Transimission complete
{
    txISR(&dmxPorts[3]);
}
