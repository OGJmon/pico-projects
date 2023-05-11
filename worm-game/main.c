#include <stdlib.h>
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "class/cdc/cdc_device.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "Adafruit_ST7735.h"
#include "graphics.h"
#include "keypad.h"
#include "player.h"
#include "field_internal.h"
#include "debug.h"
#include "tft_stdout.h"
#include "red.h"
#include "green.h"
#include "moves.h"
#include "rnd.h"
#include "bootrom_api.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

int main(void)
{
	char c;
	uint32_t i, j;
	struct player green={0,0,MOVE_NONE,1};
	struct player red={0,0,MOVE_NONE,1};

	stdio_init_all();
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

	// Set up TFT
	Adafruit_ST7735_gpio_init();
	Adafruit_ST7735_initR(INITR_18GREENTAB);
	Adafruit_ST7735_setRotation(0);
	Adafruit_ST7735_fillScreen(Adafruit_ST7735_Color565(128,128,128));
	graphics_init(128,160);
	graphics_setTextWrap(false);
	graphics_setTextColor(0xFFFF);
	graphics_setTextSize(1);

	// Borders of playing field on TFT
	// North
	Adafruit_ST7735_fillRect(0,0,128,2,Adafruit_ST7735_Color565(0,0,255));
	// West
	Adafruit_ST7735_fillRect(0,0,2,140,Adafruit_ST7735_Color565(0,0,255));
	// South
	Adafruit_ST7735_fillRect(0,138,128,2,Adafruit_ST7735_Color565(0,0,255));
	// East
	Adafruit_ST7735_fillRect(126,0,2,140,Adafruit_ST7735_Color565(0,0,255));

	// Put up field edge lines
	for (i=0;i<FIELD_WIDTH;i++) {
		field[i][0]=FIELD_EDGE;
		field[i][1]=FIELD_EDGE;
		field[i][FIELD_HEIGHT-1]=FIELD_EDGE;
		field[i][FIELD_HEIGHT-2]=FIELD_EDGE;
	}
	for (i=0;i<FIELD_HEIGHT;i++) {
		field[0][i]=FIELD_EDGE;
		field[1][i]=FIELD_EDGE;
		field[FIELD_WIDTH-1][i]=FIELD_EDGE;
		field[FIELD_WIDTH-2][i]=FIELD_EDGE;
	}
	// Set up players
	green.x=rnd()%124+2;
	green.y=rnd()%124+2;
	red.x=rnd()%124+2;
	red.y=rnd()%124+2;

	// Claim starting locations
	field[green.x][green.y]=GREEN_CAPTURE;
	field[red.x][red.y]=RED_CAPTURE;
	update(&green,&red);
	sleep_ms(1000);
    while (true) {
		green.move=green_player(green.x,green.y,red.x,red.y);
		red.move=red_player(red.x,red.y,green.x,green.y);
		rules(&green,&red);
		update(&green,&red);

		// House keeping chores
		if (tud_cdc_available()) {
			c=getchar();
		}
		switch (c) {
			case 'f':
				reset_usb_boot(0,0);
				break;
			case 'p':
				field_print();
				break;
			case 'r':
				debug=debug^DEBUG_RED;
				break;
		}
		jump_to_MSD();
		c=getkey();
		if (c>0) {
			c=mapkey(c);
			if (c==keyC) {
				Adafruit_ST7735_fillRect(2,2,124,136,Adafruit_ST7735_Color565(128,128,128));
				green.move=MOVE_NONE;
				green.score=1;
				red.move=MOVE_NONE;
				red.score=1;
				// Set up players
				green.x=rnd()%124+2;
				green.y=rnd()%124+2;
				red.x=rnd()%124+2;
				for (i=0;i<FIELD_WIDTH;i++) {
					for (j=0;j<FIELD_HEIGHT;j++) {
						field[i][j]=0;
					}
				}
				// Put up field edge lines
				for (i=0;i<FIELD_WIDTH;i++) {
					field[i][0]=FIELD_EDGE;
					field[i][1]=FIELD_EDGE;
					field[i][FIELD_HEIGHT-1]=FIELD_EDGE;
					field[i][FIELD_HEIGHT-2]=FIELD_EDGE;
				}
				for (i=0;i<FIELD_HEIGHT;i++) {
					field[0][i]=FIELD_EDGE;
					field[1][i]=FIELD_EDGE;
					field[FIELD_WIDTH-1][i]=FIELD_EDGE;
					field[FIELD_WIDTH-2][i]=FIELD_EDGE;
				}
				red.y=rnd()%124+2;

				// Claim starting locations
				field[green.x][green.y]=GREEN_CAPTURE;
				field[red.x][red.y]=RED_CAPTURE;
				update(&green,&red);
				sleep_ms(1000);
			}
		}
		c=0;
    }
}
