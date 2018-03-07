#include "display.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define Takt_TWI 400000
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
#define TWI_BAUDRATE TWI_BAUD(F_CPU, Takt_TWI)


uint8_t displayfunction = 0x00;
uint8_t displaycontrol  = 0x00;
uint8_t displaymode = 0x00;
uint8_t backlightVal = LCD_BACKLIGHT;

void setupTWI(void) {
	TWIC.MASTER.BAUD = TWI_BAUDRATE;

	TWIC.MASTER.CTRLA = TWI_MASTER_WIF_bm | TWI_MASTER_ENABLE_bm | TWI_MASTER_INTLVL_gm; //Enable Write Interupt Flag, Enable Master, Interrupt Level 0

	//RESET Bus State
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}


void twiWrite(unsigned char byte) {
	TWIC.MASTER.ADDR = LCD_ADDR;
	while((TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) == 0);
	
	TWIC.MASTER.DATA = byte;
	while((TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) == 0);
}


void writeStr(char value[]) {
    while(*value != 0) {
        send(*value++, Rs);
    }
}

void writeInt(uint8_t value) {
	char str[4];

	itoa(value, str, 10);
    writeStr(str);
}

void writeIntWidth(uint8_t value, uint8_t width) {
	char str[4];

	itoa(value, str, 10);
	
	if(width >= 3 && value < 100) {
		write(' ');
	}

	if(width >= 2 && value < 10) {
		write(' ');
	}

    writeStr(str);
}


void write(uint8_t value) {
  send(value, Rs);
}

void writeDirect(uint8_t b) {
  twiWrite(b | backlightVal);
}

void pulseEnable(uint8_t _data){
  writeDirect(_data | En);  // En high
  _delay_ms(1);   // enable pulse must be >450ns
  
  writeDirect(_data & ~En); // En low
  _delay_ms(1);    // commands need > 37us to settle
} 

void write4bits(uint8_t data) {
  writeDirect(data);
  pulseEnable(data);
}

void send(uint8_t value, uint8_t mode) {
  uint8_t highnib=value&0xf0;
  uint8_t lownib=(value<<4)&0xf0;
  write4bits((highnib)|mode);
  write4bits((lownib)|mode); 
}

void command(uint8_t value) {
  send(value, 0);
}



/**
 * Setup the Display
 */
void setupDisplay(void) {
	displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;


	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	//_delay_ms(1000);
  
	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	_delay_ms(5); // wait min 4.1ms

	// second try
	write4bits(0x03 << 4);
	_delay_ms(5); // wait min 4.1ms

	// third go!
	write4bits(0x03 << 4); 
	_delay_ms(1);

	// finally, set to 4-bit interface
	write4bits(0x02 << 4); 

	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | displayfunction);  

	// turn the display on with no cursor or blinking default
	displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	command(LCD_DISPLAYCONTROL | displaycontrol);


	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	_delay_ms(20);  // this command takes a long time!

	// Initialize to default text direction (for roman languages)
	displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	command(LCD_ENTRYMODESET | displaymode);

	command(LCD_RETURNHOME);  // set cursor position to zero
	_delay_ms(20);  // this command takes a long time!

    backlightVal = LCD_BACKLIGHT;
}

void setCursor(uint8_t col, uint8_t row) {
	int numlines = 2;
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > numlines ) {
		row = numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void clearDisplay(void) {
    command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	_delay_ms(20);  // this command takes a long time!
}