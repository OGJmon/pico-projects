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

//Enumeration for special keys
enum {left_serve, move_right, right, right_miss, right_serve, move_left, left, left_miss, backspace=10, pound, keyA, keyB, keyC, keyD};

#define LEFT_BUTTON 1
#define RIGHT_BUTTON 12 
#define LEFT_LIGHT (1<<22)
#define RIGHT_LIGHT (1<<13)
#define ALL_LIGHTS (0x3ff<<13)

// Prototypes
void init();
void check_boot();
signed char getkey(void);
signed char mapkey(signed char);

int main(void)
{
	// Set up initial state and scores
	int state = right_serve;	
	int left_score=0, right_score=0;
	
	// Iterators
	int light, i;

	// Initialize bargraph and keypad
	init();
	
	// Startup instructions	
	printf("Player 1 use '1'\n");
	printf("Player 2 use 'A'\n");

    	while (true) {
		check_boot();
		jump_to_MSD();	
	
		// Ping-pong state machine	
		switch(state) {
			case left_serve:
				sio_hw->gpio_out |= LEFT_LIGHT;
				if (mapkey(getkey()) == LEFT_BUTTON) {
					state = move_right;
				}
				break;

			case move_right:
				for (light=LEFT_LIGHT; light>RIGHT_LIGHT; light=light>>1) {
					sio_hw->gpio_out |= light;
					sleep_ms(200);
					sio_hw->gpio_out &= ~light;

					// Check early swing
					if (mapkey(getkey()) == RIGHT_BUTTON) {
						printf("Player 2 early swing!\n"); 
						state = right_miss;
						break;
					}
				}
				if (state != right_miss) state = right;
				break;

			case right:
				sio_hw->gpio_out |= RIGHT_LIGHT;
				for (i=0; i<4000; i++){
					if (mapkey(getkey()) == RIGHT_BUTTON) {
						state = move_left;
						break;
					}
				}
				if (state != move_left) state = right_miss;
				sio_hw->gpio_out &= ~RIGHT_LIGHT;
				break;

			case right_miss:
				left_score++;
				state = left_serve;
				printf("-----------------------\n");
				printf("    %d     :     %d    \n", left_score, right_score);
				printf("-----------------------\n");	
				break;
				
			case right_serve:
				sio_hw->gpio_out |= RIGHT_LIGHT;
				if (mapkey(getkey()) == RIGHT_BUTTON) {
					state = move_left;
				}
				break;
				
			case move_left:
				for (light=RIGHT_LIGHT; light<LEFT_LIGHT; light=light<<1) {
					sio_hw->gpio_out |= light;
					sleep_ms(200);
					sio_hw->gpio_out &= ~light;

					// Check early swing
					if (mapkey(getkey()) == LEFT_BUTTON) {
						printf("Player 1 early swing!\n"); 
						state = left_miss;
						break;
					}
				}
				if (state != left_miss) state = left;
				break;
				
			case left:
				sio_hw->gpio_out |= LEFT_LIGHT;
				for (i=0; i<4000; i++){
					if (mapkey(getkey()) == LEFT_BUTTON) {
						state = move_right;
						break;
					}
				}
				if (state != move_right) state = left_miss;
				sio_hw->gpio_out &= ~LEFT_LIGHT;
				break;

			case left_miss:
				right_score++;
				state = right_serve;
				printf("-----------------------\n");
				printf("    %d     :     %d    \n", left_score, right_score);
				printf("-----------------------\n");	
				break;
		}	
	



		/* --------------------------- */
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

		for (row = 0; row < 4; row++) {

		// Check current row
			if ((sio_hw->gpio_in) & (0x10 << (5+row))) {
				sio_hw->gpio_oe &= ~(0xFF<<5); // Button press found: clean up GPIOs
				return ((4 * row) + column);   // Return key value
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
	uint32_t i;

	//stdio_init_all();
	stdio_set_driver_enabled(&stdio_tft, true);
	sleep_ms(1000);

	// Onboard LED
	if (cyw43_arch_init()) {
		printf("WiFi init failed");
	}

	// Keypad 	
	for (i=5;i<13;i++) {
		gpio_init(i);
		gpio_put(i,1);
		gpio_set_dir(i,GPIO_IN);
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


}

void check_boot(void)
{
	char c = 0;

	if (tud_cdc_available()) {
		c=getchar();
	}
	switch (c) {
		case 'f':
			reset_usb_boot(0,0);
			break;
	}
}
	


