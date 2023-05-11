#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "class/cdc/cdc_device.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
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


struct note {
	unsigned char notenum;		// index into array of frequencies for this note
	unsigned char duration;		// duration of note in 1/8ths of a second
};

struct song {
	unsigned char numnotes;		// Number of notes in this song
	const struct note thenotes[40];	// Array of the notes (max 30)
};

const struct song steinsong = {
    14,
	{
		{2,12},	//	 First line of "Maine Stein Song"
		{4,1},	//	 Data is note number and duration (1/8 ths second)
		{2,3},
		{1,1},
		{2,3},
		{7,4},
		{11,8},
		{14,4},
		{7,2},
		{7,2},
		{9,4},
		{7,4},
		{6,8},
		{4,8}
	}
};

const struct song marylamb = {
    26,
	{
		{4,3},	//	 First line of "Mary Had a Little Lamb"
		{2,1},	//	 Data is note number and duration (1/8 ths second)
		{0,2},
		{2,2},
		{4,2},
		{4,2},
		{4,4},
		{2,2},
		{2,2},
		{2,4},
		{4,2},
		{7,2},
		{7,4},
		{4,3},
		{2,1},
		{0,2},
		{2,2},
		{4,2},
		{4,2},
		{4,2},
		{4,2},
		{2,2},
		{2,2},
		{4,2},
		{2,2},
		{0,8}
	}
};

const struct song rickroll = {
    30,
	{
		{0, 1}, // 	First line of "Never Gonna Give You Up" 
		{2, 1},	//	Data is note number and duration (1/8 ths second)
		{5, 1},
		{2, 1},

		{9, 4},
		{9, 4},
		{7, 4},
		{REST, 4},
	
		{0, 1},
		{2, 1},	
		{5, 1},
		{2, 1},
		
		{7, 4},		
		{7, 4},		
		{5, 4},
		{REST, 4},
	
		{0, 1},
		{2, 1},	
		{5, 1},
		{2, 1},

		{5, 6},
		{7, 2},		
		{9, 3},	

		{0, 4},
		{0, 2},

		{7, 4},
		{5, 4}		
	}
};

const unsigned int frequency[] = {        // note number & note
		262,      // 0  C
		277,      // 1  C#
		294,      // 2  D
		311,      // 3  D#
		330,      // 4  E
		349,      // 5  F
		370,      // 6  F#
		392,      // 7  G
		415,      // 8  G#
		440,      // 9  A
		466,      // 10 A#
		494,      // 11 B
		523,      // 12 C
		554,      // 13 C#
		587,       // 14 D
		0
};

// Prototypes
void init();
void check_boot();
signed char getkey(void);
signed char mapkey(signed char);
unsigned int freq_to_wrap(unsigned int, uint32_t);
void playnote(struct note);
void playsong(struct song);

int main(void)
{
	// Initialize keypad, speaker and PWM
	init();

    	while (true) {
		check_boot();
		jump_to_MSD();	
		
		// Check keypad to play songs
		if (mapkey(getkey()) == 1) { 
			printf("Play Marylamb!\n");
			playsong(marylamb);
		}
		if (mapkey(getkey()) == 2) { 
			printf("Play Stiensong!\n");
			playsong(steinsong);
		}
		if (mapkey(getkey()) == 3) { 
			printf("Play Rick Roll!\n");
			playsong(rickroll);
		}
	}
}


void playsong(struct song a) {
	int note;

	// Play each note of song
	for (note = 0; note < a.numnotes; note++) {
		playnote(a.thenotes[note]);
		sleep_ms(62);
	}
}

void playnote(struct note n) {
	
	uint32_t slice, channel, wrapper;	
	
	// Get PWM slice and channel for speaker pin
	slice = pwm_gpio_to_slice_num(SPEAKER); 
	channel = pwm_gpio_to_channel(SPEAKER);

	// Convert get frequency and convert to wrapper value
	unsigned int freq = frequency[n.notenum];
	wrapper = freq_to_wrap(freq, CLOCK_FREQ);
	
	// Set wrapper and level
	pwm_set_wrap(slice, wrapper);
	pwm_set_chan_level(slice, channel, wrapper/2);

	// Play note for desired time
	pwm_set_enabled(slice, true);
	sleep_ms(n.duration*EIGHTH_SEC);
	pwm_set_enabled(slice, false);
	
}

// Used to convert a desired frequency to
// a wrapper value based for PWM output
unsigned int freq_to_wrap(unsigned int freq, uint32_t clock_freq) {
	unsigned int wrapper;
	wrapper = clock_freq / freq;
	return wrapper;
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
	uint32_t i, slice;

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

	// Initialize to output and set function to PWM
	gpio_init(SPEAKER);
	gpio_set_dir(SPEAKER,GPIO_OUT);
	gpio_set_function(SPEAKER, GPIO_FUNC_PWM);

	// Divide PWM clock from 133Mhz -> 1.9MHz
	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv_int(&config, 70);

	// Initialize configuration to hardware
	slice = pwm_gpio_to_slice_num(SPEAKER); 
	pwm_init(slice, &config, false);

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

	if (tud_cdc_available()) {
		c=getchar();
	}
	switch (c) {
		case 'f':
			reset_usb_boot(0,0);
			break;
	}
}
	


