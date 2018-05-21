#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h> //memcpy

#include "system.h"
#include "timer.h"
#include "display.h"
#include "enc28j60.h"
#include "dmx.h"
#include "artnet.h"

#include "EtherCard.h"

uint8_t Ethernet::buffer[590];


BufferFiller bfill;

volatile uint8_t missedFrames = 0;

static uint16_t homePage() {
	bfill = ether.tcpOffset();

    char* name = "ArtNetNode 1";
	char* ip = "2.0.0.10";
	uint8_t portA = 1;
	uint8_t portB = 2;
	uint8_t portC = 3;
	uint8_t portD = 4;

	bfill.emit_p(PSTR("<!DOCTYPE html><html lang=\"de\"><head><meta charset=\"utf-8\" /><title>ArtNet Node</title><style>html,body{margin:20px;font-family:Arial;background:#eff0f1;color:#fff}#main{width:400px;margin:auto;padding:10px;background:#505659;font-weight:bold;text-align:center;border-bottom-left-radius:4px;border-bottom-right-radius:4px;font-size:13px}#header{border-top-left-radius:4px;border-top-right-radius:4px;width:400px;margin:auto;padding:10px;background:#0e89f6}input{background:#3a3f41;border:none;border-radius:2px;padding:8px;color:#fff;text-align:center;width:90%}input[type=submit]{width:100%;background:#575d60;border:#62696c 1px solid }.num{width:40px}td{text-align:left}td.c{text-align:right}</style></head><body><div id=\"header\"> Settings</div><div id=\"main\"><form method=\"POST\"> <br><table width=\"400px\" ><tr><td >IP</td><td class=\"c\" width=\"80%\"><input type=\"text\" value=\"$S\" name=\"ip\"></td></tr><tr><td >Name</td><td class=\"c\" width=\"80%\"><input type=\"text\" value=\"$S\" name=\"name\"></td></tr><tr><td >Port A</td><td class=\"c\"><input type=\"text\" class=\"num\" value=\"$D\" name=\"a\"></td></tr><tr><td>Port B</td><td class=\"c\"><input type=\"text\" class=\"num\" value=\"$D\" name=\"b\"></td></tr><tr><td>Port C</td><td class=\"c\"><input type=\"text\" class=\"num\" value=\"$D\" name=\"c\"></td></tr></table> <br> <input type=\"submit\" value=\"Save\" /></form></div></body></html>")
		,name, ip, portA, portB, portC, portD);	  
	return bfill.position();
}



//TODO: Move WebPage Stuff into other module
int main (void) {
	cli();/* disable interrupts */ 
	setUp32MhzInternalOsc();

	setupTimer0();

	//Setup debug leds
	PORTA.DIR = 0x03;
	PORTA.OUTCLR = 0x03;

	setupTWI();
	setupDisplay();

	setCursor(0,0);
	writeStr("Setup Eth0");

	static uint8_t macaddr[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };
	//TODO: Load from EEPROM
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
	ether.staticSetup(ipaddr); 
	ether.enableBroadcast();
	ether.udpServerListenOnPort(artnet.udpArtNet, UDP_PORT_ARTNET);

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


	uint8_t receivedPost = 0;

	while(1) {
		uint16_t pos = ether.packetLoop(ether.packetReceive());

		//Handle TCP packages
		if(pos) {
			char* data = (char *) Ethernet::buffer + pos;

			if (strncmp("GET / ", data, 7) == 0) {
    			ether.httpServerReply(homePage()); // send web page data
			} else if (strncmp("POST / ", data, 7) == 0) {
				receivedPost = 1;

				char result[5]; result[0] = 0;
				EtherCard::findKeyVal(data, result, 4, "a");
				/* setCursor(0,0);		
				writeStr(result);*/

    			ether.httpServerReply(homePage()); // send web page data
			}
		}

		if(ether.isReceiveError()) {
			PORTA.OUTTGL = 0x02;
			missedFrames++;
		}


		if(counter == 0) {
			counter = 12000;

			setCursor(15,0);
			if(ether.isLinkUp()) {
				writeStr("L");
			} else {
				writeStr(" ");
			}
			
			setCursor(15,1);
			if(receivedPost == 1) {
				writeStr("P");
			} else {
				writeStr(" ");
			}

			setCursor(0,1);
			writeStr("FPS ");
			writeIntWidth( artnet.getUniverseFPS(0), 2);
			writeStr(" ");
			writeIntWidth( artnet.getUniverseFPS(1), 2);
			writeStr(" ");
			writeIntWidth( artnet.getUniverseFPS(2), 2);
			writeStr(" ");
			writeIntWidth( artnet.getUniverseFPS(3), 2);
		}
		
		counter--;

	}

	return 0;
}

