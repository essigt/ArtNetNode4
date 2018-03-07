#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "system.h"
#include "timer.c"
#include "display.h"
#include "enc28j60.h"
#include "dmx.h"
#include "udp.h"


uint8_t inv = 0;

int main (void) {
	cli();/* disable interrupts */ 
	setUp32MhzInternalOsc();

	setupTimer0();

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

	uint16_t counter = 0;

	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;	

	sei();

	setCursor(0,0);
	writeStr("DMX  1  2  3  4");

	while(1) {
		uint16_t len = packetReceive();
		packageLoop(len);

		
		/*if(counter == 0) {
			setCursor(0,1);
			if(inv == 0) {
				write('|');
				inv = 1;
			} else if(inv == 1){
				write('\\');
				inv = 2;
			} else if(inv == 2){
				write('-');
				inv = 3;
			} else {
				write('/');
				inv = 0;
			}
		}*/

		if(counter == 0) {
			counter = 12000;

			setCursor(0,1);
			writeStr("FPS ");
			writeIntWidth( getUniverseFPS(0), 2);
			
			writeStr(" ");
			writeIntWidth( getUniverseFPS(1), 2);
			writeStr(" ");
			writeIntWidth( getUniverseFPS(2), 2);
			writeStr(" ");
			writeIntWidth( getUniverseFPS(3), 2);
		}
		
		counter--;

	}

	return 0;
}

