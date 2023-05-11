// A. Sheaff 2023/01/05
// Keypad routines

#include <stdio.h>
#include <pico/stdlib.h>
#include "hardware/gpio.h"
#include "keypad.h"

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

	// Drive one column high and then check every row
	for (column = 0; column <= 3; column++) {
		sio_hw->gpio_clr=(0xFF<<5);	// All outputs zero for keypad
		sio_hw->gpio_set=0x01<<(5+column);	// One column high
		sio_hw->gpio_oe_clr=0xFF<<5;	// All inputs
		sio_hw->gpio_oe_set=0x01<<(column+5);	// ONE output
		sleep_ms(1);	// Wait for SIO to propigate
		// Check over all the rows for a pressed key
		for (row = 0; row <= 3; row++) {
			if (((sio_hw->gpio_in)&(0xF0<<5)) & (0x10 << (5+row))) {
				// Clean up GPIOs
				sio_hw->gpio_clr=(0xFF<<5);
				sio_hw->gpio_oe_clr=0xFF<<5;
				// Return key value
				return ((4 * row) + column);
			}
		}
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
