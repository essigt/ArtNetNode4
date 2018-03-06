#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "system.h"
#include "display.h"
#include "enc28j60.h"
#include "dmx.h"



int main (void) {
	cli();/* disable interrupts */ 
	setUp32MhzInternalOsc();

	//Setup debug leds
	PORTA.DIR = 0x03;
	PORTA.OUTSET = 0x03;

	setupTWI();
	setupDisplay();

	setCursor(0,0);
	writeStr("Setup Eth0");
	setupEthernet();

	clearDisplay();

	setCursor(0,0);
	writeStr("Setup DMX");
	setupDMX();

	dmxBuffer[0][0] = 128;
	dmxBuffer[0][1] = 0xFF;
	dmxBuffer[0][2] = 0x0F;

	uint8_t counter = 0;

	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;	

	sei();

	while(1) {
		uint16_t len = packetReceive();
		packageLoop(len);

		startDMX_TX(); //Reicht? TODO: Timer, 44mal/sec


		if(counter == 0) {
			counter = 2000;
			setCursor(0,0);
			writeStr("Ch1:");
			writeInt(dmxBuffer[0][0]);
			writeStr(" Ch2:");
			writeInt(dmxBuffer[0][1]);
			writeStr("     ");

			
		}
		
		counter--;

	}

	return 0;
}

