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

#define KEY_GPIO_START 5
#define KEY_NUMBER_GPIOS 8

// Enumeration for special keys
enum {backspace=10, pound, keyA,keyB,keyC,keyD};

// Initialize the keypad GPIOs
void keypad_init(void)
{
	uint32_t i;

	for (i=KEY_GPIO_START;i<KEY_GPIO_START+KEY_NUMBER_GPIOS;i++) {
		gpio_init(i);
		gpio_put(i,1);
		gpio_set_dir(i,GPIO_IN);
	}
}

// Scan for a key press.  Return -1 if not key is pressed
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

		//printf("%02x\n", 0xFF & (sio_hw->gpio_in >> 5));

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

// Get a number - no backspace
uint32_t getnum(void)
{
	uint32_t v=0;	// The value
	signed char c;	// The key press

	do {
		c=getkey();
		if (c>=0) {
			c=mapkey(c);
			if ((c>=0) && (c<=9)) {	// Number
				v=10*v+c;
				printf("%c",'0'+c);
			}
			while (getkey()!=-1);	// Wait for release
			sleep_ms(25);			// Debounce
		}
	} while (c!=pound);	// '#' exits/enters
	printf("\n");

	return v;
}

// Get a number with backspace
uint32_t getnumbs(void)
{
	uint32_t v=0;	// The value
	signed char c;	// The key press

	do {
		c=getkey();
		if (c>=0) {
			c=mapkey(c);
			if ((c>=0) && (c<=9)) {	// Number
				v=10*v+c;
				printf("%c",'0'+c);
			} else if (c==backspace) {
				v=v/10;
				printf("\b");
			}
			while (getkey()!=-1);	// Wait for release
			sleep_ms(25);			// Debounce
		}
	} while (c!=pound);	// Exit on '#'/Enter
	printf("\n");

	return v;

}

int main(void)
{
	char c;
	uint32_t i;
	signed char d, b;
	uint32_t bar=1<<13, delay = 10;
	bool dir=0;

	//stdio_init_all();
	stdio_set_driver_enabled(&stdio_tft, true);

	sleep_ms(1000);

	// Keypad init
	keypad_init();

	// Onboard LED
	if (cyw43_arch_init()) {
		printf("WiFi init failed");
	}

	// Bargraph
	for (i=13;i<13+10;i++) {
		gpio_init(i);
		gpio_put(i,0);
		gpio_set_dir(i,GPIO_OUT);
	}

	Adafruit_ST7735_gpio_init();
	Adafruit_ST7735_initR(INITR_18GREENTAB);
	Adafruit_ST7735_setRotation(0);
	Adafruit_ST7735_fillScreen(0x0000);
	graphics_init(128,160);
	graphics_setTextWrap(false);
	graphics_setTextColor(0xFFFF);
	graphics_setTextSize(1);
		
    while (true) {
		if (tud_cdc_available()) {
			c=getchar();
		}
		switch (c) {
			case 'f':
				reset_usb_boot(0,0);
				break;
		}
		
		// If light is on edge of bar graph change direction
		if ((bar >= 1 << 22) || (bar <= 1 << 13)) dir = !dir;
		
		// clear bar variable
		sio_hw->gpio_out &= ~bar;

		// Check direction
		if (dir) {
			bar = bar << 1;
		}
		else {
			bar = bar >> 1;
		}

		// turn on next light
		sio_hw->gpio_out |= bar;
		
		/******************** Print Key  ***********************/
		d=getkey();
		if (d>=0) {
			b = mapkey(d);
			printf("%d %d\n",d,b);
		
			if (b == 1) {
				delay += 10;		
			}
			else if ((b == 2) && (delay > 10)) {
				delay -= 10;
			}
			else if (b == 3) {
				// debounce
				b = 0;
				printf("PAUSE\n");
				sleep_ms(150);

				// Wait for button to play again
				while(b != 3) {
					b = mapkey(getkey());
				}
			
				b = 0;	
				printf("PLAY\n");
				sleep_ms(150);
			}
		}
		else {
			printf("%d\n",d);
		}

		// look for MSD keys to be pressed
		for (i=0;i<delay;i++) {	
				jump_to_MSD();
		}
    }
}
