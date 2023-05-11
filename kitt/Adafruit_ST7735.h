/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifndef ADAFRUIT_ST7735H
#define ADAFRUIT_ST7735H

#include <stdint.h>
#include <stdbool.h>

#define LCD_TX_PIN 3	// TX MOSI
#define LCD_RX_PIN 0	// RX MISO
#define LCD_CK_PIN 2	// Clock
#define LCD_DC_PIN 4	// Data/Command
#define LCD_BL_PIN 27	// Backlight
#define LCD_RT_PIN 26	// Reset
#define LCD_CS_PIN 1	// Chip Select

void Adafruit_ST7735_gpio_init(void);

// some flags for initR() :(
#define INITR_GREENTAB   0x0
#define INITR_REDTAB     0x1
#define INITR_BLACKTAB   0x2

#define INITR_18GREENTAB    INITR_GREENTAB
#define INITR_18REDTAB      INITR_REDTAB
#define INITR_18BLACKTAB    INITR_BLACKTAB
#define INITR_144GREENTAB   0x1
#define INITR_MINI160x80    0x4


// for 1.44 and mini
#define ST7735_TFTWIDTH_128  128
// for mini
#define ST7735_TFTWIDTH_80   80
// for 1.44" display
#define ST7735_TFTHEIGHT_128 128
// for 1.8" and mini display
#define ST7735_TFTHEIGHT_160  160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF


void Adafruit_ST7735_initB(void);	// for ST7735B displays
void Adafruit_ST7735_initR(uint8_t options);	// for ST7735R
void Adafruit_ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void Adafruit_ST7735_pushColor(uint16_t color);
void Adafruit_ST7735_fillScreen(uint16_t color);
void Adafruit_ST7735_drawPixel(int16_t x, int16_t y, uint16_t color);
void Adafruit_ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void Adafruit_ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void Adafruit_ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void Adafruit_ST7735_setRotation(uint8_t r);
void Adafruit_ST7735_invertDisplay(bool i);
uint16_t Adafruit_ST7735_Color565(uint8_t r, uint8_t g, uint8_t b);

void Adafruit_ST7735_spiwrite(uint8_t);
void Adafruit_ST7735_writecommand(uint8_t c);
void Adafruit_ST7735_writedata(uint8_t d);
void Adafruit_ST7735_commandList(const uint8_t * addr);
void Adafruit_ST7735_commonInit(const uint8_t * cmdList);
void Adafruit_ST7735_writefb(uint16_t *fb);


#endif
