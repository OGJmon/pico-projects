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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "Adafruit_ST7735.h"

static uint8_t  tabcolor, rotation;
static uint8_t colstart, rowstart, xstart, ystart; // some displays need this changed
static uint16_t _width, _height;

void Adafruit_ST7735_gpio_init(void)
{
	spi_init(spi0, 15 * 1000 * 1000);
	gpio_set_function(LCD_CK_PIN, GPIO_FUNC_SPI);	// Clock
	gpio_set_function(LCD_TX_PIN, GPIO_FUNC_SPI);	// TX MOSI
	gpio_set_function(LCD_RX_PIN, GPIO_FUNC_SPI);	// RX MISO

	gpio_init(LCD_CS_PIN);	// CS
	gpio_set_dir(LCD_CS_PIN,GPIO_OUT);
	gpio_put(LCD_CS_PIN,1);

	gpio_init(LCD_DC_PIN);	// D/C
	gpio_set_dir(LCD_DC_PIN,GPIO_OUT);
	gpio_put(LCD_DC_PIN,0);

	gpio_init(LCD_BL_PIN);	// BL
	gpio_set_dir(LCD_BL_PIN,GPIO_OUT);
	gpio_put(LCD_BL_PIN,1);

	gpio_init(LCD_RT_PIN);	// Reset
	gpio_set_dir(LCD_RT_PIN,GPIO_OUT);
	gpio_put(LCD_RT_PIN,0);
}

/******** low level bit twiddling **********/


void Adafruit_ST7735_CS_HIGH(void)
{
	gpio_put(LCD_CS_PIN, 1);
}

void Adafruit_ST7735_CS_LOW(void)
{
	gpio_put(LCD_CS_PIN, 0);
}

void Adafruit_ST7735_DC_HIGH(void)
{
	gpio_put(LCD_DC_PIN, 1);
}

void Adafruit_ST7735_DC_LOW(void)
{
	gpio_put(LCD_DC_PIN, 0);
}

inline uint16_t swapcolor(uint16_t x)
{
	return (x << 11) | (x & 0x07E0) | (x >> 11);
}

inline void Adafruit_ST7735_spiwrite(uint8_t c)
{
	spi_write_blocking(spi0,&c,1);
}

void Adafruit_ST7735_writecommand(uint8_t c)
{
	Adafruit_ST7735_DC_LOW();
	Adafruit_ST7735_CS_LOW();

	Adafruit_ST7735_spiwrite(c);

	Adafruit_ST7735_CS_HIGH();
}

void Adafruit_ST7735_writedata(uint8_t c)
{
	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();

	Adafruit_ST7735_spiwrite(c);

	Adafruit_ST7735_CS_HIGH();
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t Bcmd[] = {	// Initialization commands for 7735B screens
	18,			// 18 commands in list:
	ST7735_SWRESET, DELAY,	//  1: Software reset, no args, w/HAL_Delay
	50,			//     50 ms HAL_Delay
	ST7735_SLPOUT, DELAY,	//  2: Out of sleep mode, no args, w/HAL_Delay
	255,			//     255 = 500 ms HAL_Delay
	ST7735_COLMOD, 1 + DELAY,	//  3: Set color mode, 1 arg + HAL_Delay:
	0x05,			//     16-bit color
	10,			//     10 ms HAL_Delay
	ST7735_FRMCTR1, 3 + DELAY,	//  4: Frame rate control, 3 args + HAL_Delay:
	0x00,			//     fastest refresh
	0x06,			//     6 lines front porch
	0x03,			//     3 lines back porch
	10,			//     10 ms HAL_Delay
	ST7735_MADCTL, 1,	//  5: Memory access ctrl (directions), 1 arg:
	0x08,			//     Row addr/col addr, bottom to top refresh
	ST7735_DISSET5, 2,	//  6: Display settings #5, 2 args, no HAL_Delay:
	0x15,			//     1 clk cycle nonoverlap, 2 cycle gate
	//     rise, 3 cycle osc equalize
	0x02,			//     Fix on VTL
	ST7735_INVCTR, 1,	//  7: Display inversion control, 1 arg:
	0x0,			//     Line inversion
	ST7735_PWCTR1, 2 + DELAY,	//  8: Power control, 2 args + HAL_Delay:
	0x02,			//     GVDD = 4.7V
	0x70,			//     1.0uA
	10,			//     10 ms HAL_Delay
	ST7735_PWCTR2, 1,	//  9: Power control, 1 arg, no HAL_Delay:
	0x05,			//     VGH = 14.7V, VGL = -7.35V
	ST7735_PWCTR3, 2,	// 10: Power control, 2 args, no HAL_Delay:
	0x01,			//     Opamp current small
	0x02,			//     Boost frequency
	ST7735_VMCTR1, 2 + DELAY,	// 11: Power control, 2 args + HAL_Delay:
	0x3C,			//     VCOMH = 4V
	0x38,			//     VCOML = -1.1V
	10,			//     10 ms HAL_Delay
	ST7735_PWCTR6, 2,	// 12: Power control, 2 args, no HAL_Delay:
	0x11, 0x15,
	ST7735_GMCTRP1, 16,	// 13: Magical unicorn dust, 16 args, no HAL_Delay:
	0x09, 0x16, 0x09, 0x20,	//     (seriously though, not sure what
	0x21, 0x1B, 0x13, 0x19,	//      these config values represent)
	0x17, 0x15, 0x1E, 0x2B,
	0x04, 0x05, 0x02, 0x0E,
	ST7735_GMCTRN1, 16 + DELAY,	// 14: Sparkles and rainbows, 16 args + HAL_Delay:
	0x0B, 0x14, 0x08, 0x1E,	//     (ditto)
	0x22, 0x1D, 0x18, 0x1E,
	0x1B, 0x1A, 0x24, 0x2B,
	0x06, 0x06, 0x02, 0x0F,
	10,			//     10 ms HAL_Delay
	ST7735_CASET, 4,	// 15: Column addr set, 4 args, no HAL_Delay:
	0x00, 0x02,		//     XSTART = 2
	0x00, 0x81,		//     XEND = 129
	ST7735_RASET, 4,	// 16: Row addr set, 4 args, no HAL_Delay:
	0x00, 0x02,		//     XSTART = 1
	0x00, 0x81,		//     XEND = 160
	ST7735_NORON, DELAY,	// 17: Normal display on, no args, w/HAL_Delay
	10,			//     10 ms HAL_Delay
	ST7735_DISPON, DELAY,	// 18: Main screen turn on, no args, w/HAL_Delay
	255
},				//     255 = 500 ms HAL_Delay

    Rcmd1[] = {			// Init for 7735R, part 1 (red or green tab)
	15,			// 15 commands in list:
	    ST7735_SWRESET, DELAY,	//  1: Software reset, 0 args, w/HAL_Delay
	    150,		//     150 ms HAL_Delay
	    ST7735_SLPOUT, DELAY,	//  2: Out of sleep mode, 0 args, w/HAL_Delay
	    255,		//     500 ms HAL_Delay
	    ST7735_FRMCTR1, 3,	//  3: Frame rate ctrl - normal mode, 3 args:
	    0x01, 0x2C, 0x2D,	//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	    ST7735_FRMCTR2, 3,	//  4: Frame rate control - idle mode, 3 args:
	    0x01, 0x2C, 0x2D,	//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	    ST7735_FRMCTR3, 6,	//  5: Frame rate ctrl - partial mode, 6 args:
	    0x01, 0x2C, 0x2D,	//     Dot inversion mode
	    0x01, 0x2C, 0x2D,	//     Line inversion mode
	    ST7735_INVCTR, 1,	//  6: Display inversion ctrl, 1 arg, no HAL_Delay:
	    0x07,		//     No inversion
	    ST7735_PWCTR1, 3,	//  7: Power control, 3 args, no HAL_Delay:
	    0xA2, 0x02,		//     -4.6V
	    0x84,		//     AUTO mode
	    ST7735_PWCTR2, 1,	//  8: Power control, 1 arg, no HAL_Delay:
	    0xC5,		//     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
	    ST7735_PWCTR3, 2,	//  9: Power control, 2 args, no HAL_Delay:
	    0x0A,		//     Opamp current small
	    0x00,		//     Boost frequency
	    ST7735_PWCTR4, 2,	// 10: Power control, 2 args, no HAL_Delay:
	    0x8A,		//     BCLK/2, Opamp current small & Medium low
	    0x2A, ST7735_PWCTR5, 2,	// 11: Power control, 2 args, no HAL_Delay:
	    0x8A, 0xEE, ST7735_VMCTR1, 1,	// 12: Power control, 1 arg, no HAL_Delay:
	    0x0E, ST7735_INVOFF, 0,	// 13: Don't invert display, no args, no HAL_Delay
	    ST7735_MADCTL, 1,	// 14: Memory access control (directions), 1 arg:
	    0xC8,		//     row addr/col addr, bottom to top refresh
	    ST7735_COLMOD, 1,	// 15: set color mode, 1 arg, no HAL_Delay:
0x05},				//     16-bit color

    Rcmd2green[] = {		// Init for 7735R, part 2 (green tab only)
	2,			//  2 commands in list:
	    ST7735_CASET, 4,	//  1: Column addr set, 4 args, no HAL_Delay:
	    0x00, 0x02,		//     XSTART = 0
	    0x00, 0x7F + 0x02,	//     XEND = 127
	    ST7735_RASET, 4,	//  2: Row addr set, 4 args, no HAL_Delay:
	    0x00, 0x01,		//     XSTART = 0
0x00, 0x9F + 0x01},		//     XEND = 159

    Rcmd2red[] = {		// Init for 7735R, part 2 (red tab only)
	2,			//  2 commands in list:
	    ST7735_CASET, 4,	//  1: Column addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
	    0x00, 0x7F,		//     XEND = 127
	    ST7735_RASET, 4,	//  2: Row addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
0x00, 0x9F},			//     XEND = 159

    Rcmd2green144[] = {		// Init for 7735R, part 2 (green 1.44 tab)
	2,			//  2 commands in list:
	    ST7735_CASET, 4,	//  1: Column addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
	    0x00, 0x7F,		//     XEND = 127
	    ST7735_RASET, 4,	//  2: Row addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
0x00, 0x7F},			//     XEND = 127

    Rcmd2green160x80[] = {	// Init for 7735R, part 2 (mini 160x80)
	2,			//  2 commands in list:
	    ST7735_CASET, 4,	//  1: Column addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
	    0x00, 0x7F,		//     XEND = 79
	    ST7735_RASET, 4,	//  2: Row addr set, 4 args, no HAL_Delay:
	    0x00, 0x00,		//     XSTART = 0
0x00, 0x9F},			//     XEND = 159

    Rcmd3[] = {			// Init for 7735R, part 3 (red or green tab)
	4,			//  4 commands in list:
	    ST7735_GMCTRP1, 16,	//  1: Magical unicorn dust, 16 args, no HAL_Delay:
	    0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, ST7735_GMCTRN1, 16,	//  2: Sparkles and rainbows, 16 args, no HAL_Delay:
	    0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, ST7735_NORON, DELAY,	//  3: Normal display on, no args, w/HAL_Delay
	    10,			//     10 ms HAL_Delay
	    ST7735_DISPON, DELAY,	//  4: Main screen turn on, no args w/HAL_Delay
100};				//     100 ms HAL_Delay


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST7735_commandList(const uint8_t * addr)
{

	uint8_t numCommands, numArgs;
	uint16_t ms;

	numCommands = *(addr++);	// Number of commands to follow
	while (numCommands--) {	// For each command...
		Adafruit_ST7735_writecommand(*(addr++));	//   Read, issue command
		numArgs = *(addr++);	//   Number of args to follow
		ms = numArgs & DELAY;	//   If hibit set, HAL_Delay follows args
		numArgs &= ~DELAY;	//   Mask out HAL_Delay bit
		if (numArgs>0) {
			Adafruit_ST7735_DC_HIGH();
			Adafruit_ST7735_CS_LOW();
			spi_write_blocking(spi0,(uint8_t *)addr,numArgs);
			Adafruit_ST7735_CS_HIGH();
		}
		addr+=numArgs;

		if (ms) {
			ms = *(addr++);	// Read post-command HAL_Delay time (ms)
			if (ms == 255)
				ms = 500;	// If 255, HAL_Delay for 500 ms
			sleep_ms(ms);
		}
	}
}


// Initialization code common to both 'B' and 'R' type displays
void Adafruit_ST7735_commonInit(const uint8_t * cmdList)
{
	ystart = xstart = colstart = rowstart = 0;	// May be overridden in init func

	// toggle RST low to reset; CS low so it'll listen to us
	Adafruit_ST7735_CS_LOW();
	gpio_put(LCD_RT_PIN,1);
	sleep_ms(1);
	gpio_put(LCD_RT_PIN,0);
	sleep_ms(1);
	gpio_put(LCD_RT_PIN,1);
	sleep_ms(500);
	if (cmdList)
		Adafruit_ST7735_commandList(cmdList);
}


// Initialization for ST7735B screens
void Adafruit_ST7735_initB(void)
{
	Adafruit_ST7735_commonInit(Bcmd);

	Adafruit_ST7735_setRotation(0);
}


// Initialization for ST7735R screens (green or red tabs)
void Adafruit_ST7735_initR(uint8_t options)
{
	Adafruit_ST7735_commonInit(Rcmd1);
	if (options == INITR_GREENTAB) {
		Adafruit_ST7735_commandList(Rcmd2green);
		colstart = 0;
		rowstart = 0;
	} else if (options == INITR_144GREENTAB) {
		_height = ST7735_TFTHEIGHT_128;
		_width = ST7735_TFTWIDTH_128;
		Adafruit_ST7735_commandList(Rcmd2green144);
		colstart = 2;
		rowstart = 3;
	} else if (options == INITR_MINI160x80) {
		_height = ST7735_TFTHEIGHT_160;
		_width = ST7735_TFTWIDTH_80;
		Adafruit_ST7735_commandList(Rcmd2green160x80);
		colstart = 24;
		rowstart = 0;
	} else {
		// colstart, rowstart left at default '0' values
		Adafruit_ST7735_commandList(Rcmd2red);
	}
	Adafruit_ST7735_commandList(Rcmd3);

	// if black, change MADCTL color filter
	if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
		Adafruit_ST7735_writecommand(ST7735_MADCTL);
		Adafruit_ST7735_writedata(0xC0);
	}

	tabcolor = options;

	Adafruit_ST7735_setRotation(0);
}


void Adafruit_ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{

	Adafruit_ST7735_writecommand(ST7735_CASET);	// Column addr set
	Adafruit_ST7735_writedata(0x00);
	Adafruit_ST7735_writedata(x0 + xstart);	// XSTART 
	Adafruit_ST7735_writedata(0x00);
	Adafruit_ST7735_writedata(x1 + xstart);	// XEND

	Adafruit_ST7735_writecommand(ST7735_RASET);	// Row addr set
	Adafruit_ST7735_writedata(0x00);
	Adafruit_ST7735_writedata(y0 + ystart);	// YSTART
	Adafruit_ST7735_writedata(0x00);
	Adafruit_ST7735_writedata(y1 + ystart);	// YEND

	Adafruit_ST7735_writecommand(ST7735_RAMWR);	// write to RAM
}


void Adafruit_ST7735_pushColor(uint16_t color)
{
	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	Adafruit_ST7735_spiwrite(color >> 8);
	Adafruit_ST7735_spiwrite(color);
	Adafruit_ST7735_CS_HIGH();
}

void Adafruit_ST7735_drawPixel(int16_t x, int16_t y, uint16_t color)
{

	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
		return;

	Adafruit_ST7735_setAddrWindow(x, y, x + 1, y + 1);

	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	Adafruit_ST7735_spiwrite(color >> 8);
	Adafruit_ST7735_spiwrite(color);
	Adafruit_ST7735_CS_HIGH();

}


void Adafruit_ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{

	// Rudimentary clipping
	if ((x >= _width) || (y >= _height))
		return;
	if ((y + h - 1) >= _height)
		h = _height - y;
	Adafruit_ST7735_setAddrWindow(x, y, x, y + h - 1);

//	uint8_t hi = color >> 8, lo = color;

	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	uint16_t line[h];
	for (uint32_t i=0;i<h;i++) {
		line[i]=color;
	}
	spi_write_blocking(spi0,(uint8_t *)line,h*sizeof(uint16_t));
//	while (h--) {
//		Adafruit_ST7735_spiwrite(hi);
//		Adafruit_ST7735_spiwrite(lo);
//	}
	Adafruit_ST7735_CS_HIGH();
}


void Adafruit_ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{

	// Rudimentary clipping
	if ((x >= _width) || (y >= _height))
		return;
	if ((x + w - 1) >= _width)
		w = _width - x;
	Adafruit_ST7735_setAddrWindow(x, y, x + w - 1, y);

	//uint8_t hi = color >> 8, lo = color;

	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	uint16_t line[w];
	for (uint32_t i=0;i<w;i++) {
		line[i]=color;
	}
	spi_write_blocking(spi0,(uint8_t *)line,w*sizeof(uint16_t));
//	while (w--) {
//		Adafruit_ST7735_spiwrite(hi);
//		Adafruit_ST7735_spiwrite(lo);
//	}
	Adafruit_ST7735_CS_HIGH();
}



void Adafruit_ST7735_fillScreen(uint16_t color)
{
	Adafruit_ST7735_fillRect(0, 0, _width, _height, color);
}



// fill a rectangle
void Adafruit_ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{

	// rudimentary clipping (drawChar w/big text requires this)
	if ((x >= _width) || (y >= _height))
		return;
	if ((x + w - 1) >= _width)
		w = _width - x;
	if ((y + h - 1) >= _height)
		h = _height - y;

	Adafruit_ST7735_setAddrWindow(x, y, x + w - 1, y + h - 1);

	//uint8_t hi = color >> 8, lo = color;

	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	uint16_t line[w];
	for (uint32_t i=0;i<w;i++) {
		line[i]=color;
	}
	for (y = h; y > 0; y--) {
		spi_write_blocking(spi0,(uint8_t *)line,w*sizeof(uint16_t));
//		for (x = w; x > 0; x--) {
//			Adafruit_ST7735_spiwrite(hi);
//			Adafruit_ST7735_spiwrite(lo);
//		}
	}
	Adafruit_ST7735_CS_HIGH();
}

void Adafruit_ST7735_writefb(uint16_t *fb)
{
	//uint8_t hi, lo;
	//uint16_t i;
	Adafruit_ST7735_setAddrWindow(0,0,128,160);

	Adafruit_ST7735_DC_HIGH();
	Adafruit_ST7735_CS_LOW();
	spi_write_blocking(spi0,(uint8_t *)fb,160*128*sizeof(uint16_t));
//	for (i=0;i<160*128;i++) {
//		hi=(fb[i]>>8);
//		lo=(fb[i]&0xFF);
//		Adafruit_ST7735_spiwrite(hi);
//		Adafruit_ST7735_spiwrite(lo);
//	}
	Adafruit_ST7735_CS_HIGH();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST7735_Color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ST7735_setRotation(uint8_t m)
{

	Adafruit_ST7735_writecommand(ST7735_MADCTL);
	rotation = m % 4;	// can't be higher than 3
	switch (rotation) {
	case 0:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
			Adafruit_ST7735_writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
		} else {
			Adafruit_ST7735_writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
		}

		if (tabcolor == INITR_144GREENTAB) {
			_height = ST7735_TFTHEIGHT_128;
			_width = ST7735_TFTWIDTH_128;
		} else if (tabcolor == INITR_MINI160x80) {
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_80;
		} else {
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_128;
		}
		xstart = colstart;
		ystart = rowstart;
		break;
	case 1:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
			Adafruit_ST7735_writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
		} else {
			Adafruit_ST7735_writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		}

		if (tabcolor == INITR_144GREENTAB) {
			_width = ST7735_TFTHEIGHT_128;
			_height = ST7735_TFTWIDTH_128;
		} else if (tabcolor == INITR_MINI160x80) {
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_80;
		} else {
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_128;
		}
		ystart = colstart;
		xstart = rowstart;
		break;
	case 2:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
			Adafruit_ST7735_writedata(MADCTL_RGB);
		} else {
			Adafruit_ST7735_writedata(MADCTL_BGR);
		}

		if (tabcolor == INITR_144GREENTAB) {
			_height = ST7735_TFTHEIGHT_128;
			_width = ST7735_TFTWIDTH_128;
		} else if (tabcolor == INITR_MINI160x80) {
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_80;
		} else {
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_128;
		}
		xstart = colstart;
		ystart = rowstart;
		break;
	case 3:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
			Adafruit_ST7735_writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
		} else {
			Adafruit_ST7735_writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
		}

		if (tabcolor == INITR_144GREENTAB) {
			_width = ST7735_TFTHEIGHT_128;
			_height = ST7735_TFTWIDTH_128;
		} else if (tabcolor == INITR_MINI160x80) {
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_80;
		} else {
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_128;
		}
		ystart = colstart;
		xstart = rowstart;
		break;
	}
}


void Adafruit_ST7735_invertDisplay(bool i)
{
	Adafruit_ST7735_writecommand(i ? ST7735_INVON : ST7735_INVOFF);
}


