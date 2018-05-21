#ifndef _DISPLAY_H
#define _DISPLAY_H   1


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LCD_ADDR (0x3F << 1)


// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 1 << 2  // B00000100  // Enable bit
#define Rw 1 << 1  // B00000010  // Read/Write bit
#define Rs 1 << 0  // B00000001  // Register select bit


extern void setupTWI(void);
extern void twiWrite(unsigned char byte);
extern void writeStr(const  char* value);
extern void writeInt(uint8_t value);
extern void writeIntWidth(uint8_t value, uint8_t width);
extern void write(uint8_t value);

extern void setupDisplay(void);
extern void setCursor(uint8_t col, uint8_t row);

extern void clearDisplay(void);

#endif