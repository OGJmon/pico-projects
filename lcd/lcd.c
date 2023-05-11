#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "class/cdc/cdc_device.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "Adafruit_ST7735.h"
#include "graphics.h"
#include "bootrom_api.h"
#include "tft_stdout.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

#define SPEAKER 28
#define CLOCK_FREQ 1900000
#define EIGHTH_SEC 125
#define REST 15

//Enumeration for special keys
enum {backspace=10, pound, keyA, keyB, keyC, keyD};

void init();
void check_boot();
signed char getkey(void);
signed char mapkey(signed char);

int main(void)
{
	// Initialize keypad, speaker and PWM
	init();

	Adafruit_ST7735_fillRect(0, 0, 60, 40, 0x07FF);	
	Adafruit_ST7735_fillRect(0, 40, 60, 40, 0xFFFF);
	Adafruit_ST7735_fillRect(60, 0, 68, 80, 0x7BEF);

	// Team Abbreviation
	graphics_drawChar(2, 10, 77, 0xFFFF, 0x07FF, 3);
	graphics_drawChar(20, 10, 67, 0xFFFF, 0x07FF, 3);
	graphics_drawChar(38, 10, 73, 0xFFFF, 0x07FF, 3);

	// Time 
	graphics_drawChar(70, 30, 58, 0x0000, 0x7BEF, 2);
	graphics_drawChar(63, 30, 51, 0x0000, 0x7BEF, 2);
	graphics_drawChar(80, 30, 48, 0x0000, 0x7BEF, 2);
	graphics_drawChar(92, 30, 48, 0x000, 0x7BEF, 2);
	graphics_drawChar(104, 30, 80, 0x000, 0x7BEF, 2);
	graphics_drawChar(116, 30, 77, 0x000, 0x7BEF, 2);
	
	// Team Abbreviation
	graphics_drawChar(2, 50, 82, 0x0000, 0xFFFF, 3);
	graphics_drawChar(20, 50, 69, 0x0000, 0xFFFF, 3);
	graphics_drawChar(38, 50, 65, 0x0000, 0xFFFF, 3);



   	while (true) {
		check_boot();
		jump_to_MSD();
	
			
	}
}


// Scan for a key press.  Return -1 iinnnf not key is pressed
// Return 0-15 inclusive for the key pressed.  Key press value
// may not be the symbol on the keypad
signed char getkey(void)
{
	signed char column;	// Column counter
	signed char row;	// Row counter

	// *** REGISTERS ***
	// gpio_out: Used to set outputs high or low
	// gpio_oe: Used to enable output pins
	// gpio_in: Used to check value at input

	// Set all row outputs high (not enabled)
	sio_hw->gpio_out |= (0xF << 5); 

	// Drive one column high and then check every row
	for (column = 0; column < 4; column++) {
		// Set current column high
		sio_hw->gpio_oe |= (1 << (5 + column));
		sleep_us(25);
		for (row = 0; row < 4; row++) {
			// Check current row
			if ((sio_hw->gpio_in) & (0x10 << (5+row))) {
				// Button press found: clean up GPIOs
				sio_hw->gpio_oe &= ~(0xFF<<5);
				// Return key value
				return ((4 * row) + column);
			}
		}
		// No key found clean up GPIOs
		sio_hw->gpio_oe &= ~(1 << (5 + column));
	}
	return -1;	// No key pressed
}

// Map the key press to the key symbol on the key pad
signed char mapkey(signed char key)
{
	static const signed char array[] = { keyD, pound, 0, backspace, keyC, 9, 8, 7, keyB, 6, 5, 4, keyA, 3, 2, 1 };
	if (key<0) {	// Bad key outside array bounds
		return -1;
	}
	if (key>15) {	// Bad key outside array bounds
		return -1;
	}
	return array[key];
}


void init()
{
	//stdio_init_all();
	stdio_set_driver_enabled(&stdio_tft, true);
	sleep_ms(1000);

	// Onboard LED
	if (cyw43_arch_init()) {
		printf("WiFi init failed");
	}

	// Keypad 	
	for (int i=5;i<13;i++) {
		gpio_init(i);
		gpio_put(i,1);
		gpio_set_dir(i,GPIO_IN);
	}

	// Screen init
	Adafruit_ST7735_gpio_init();
	Adafruit_ST7735_initR(INITR_18GREENTAB);
	Adafruit_ST7735_setRotation(0);
	Adafruit_ST7735_fillScreen(0x0000);
	graphics_init(128,160);
	graphics_setTextWrap(false);
	graphics_setTextColor(0xFFFF);
	graphics_setTextSize(1);
}

void check_boot(void)
{
	char c = 0;

	if (tud_cdc_available()) c=getchar();

	switch (c) {
		case 'f':
			reset_usb_boot(0,0);
			break;
	}
}
	


