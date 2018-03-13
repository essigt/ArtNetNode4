#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "system.h"
#include "timer.h"
#include "display.h"
#include "enc28j60.h"
#include "dmx.h"
#include "artnet.h"

#include "ethercard.h"

uint8_t Ethernet::buffer[1500];



void udpArtNet(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) {

	int result = strncmp(data, "Art-Net",7);
	uint16_t opCode = (data[8] << 8) | data[9];
	uint16_t protocolVersion = data[10] << 8 | data[11];

	uint16_t universe = data[14] << 8 | data[15];
	uint16_t length = data[16] << 8 | data[17];

	if(opCode == 0x5000) { //ArtDMX
		PORTA.OUTTGL = 0x03;
	}
}

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

	static uint8_t macaddr[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };
	static uint8_t ipaddr[] = { 2,0,0,10 };

	setCursor(0,0);
	writeStr("IP:");
	writeInt(ipaddr[0]);
	writeStr(".");
	writeInt(ipaddr[1]);
	writeStr(".");
	writeInt(ipaddr[2]);
	writeStr(".");
	writeInt(ipaddr[3]);

	ether.begin(sizeof Ethernet::buffer, macaddr);
	ether.staticSetup(ipaddr); //TODO: GW IP?
	ether.udpServerListenOnPort(&udpArtNet, UDP_PORT_ARTNET);

	clearDisplay();

	setCursor(0,0);
	writeStr("Setup DMX");
	setupDMX();

	ether.buffer[0] = 0;

	uint16_t counter = 0;

	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;	

	sei();

	setCursor(0,0);
	writeStr("DMX  1  2  3  4");

	while(1) {
		ether.packetLoop(ether.packetReceive());


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

