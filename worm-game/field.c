#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "field_internal.h"
#include "player.h"
#include "Adafruit_ST7735.h"
#include "graphics.h"
#include "moves.h"

uint8_t field[FIELD_WIDTH][FIELD_HEIGHT]={0};

void field_print(void)
{
	uint8_t x, y;

	printf("\r\n");
	for (y=0;y<FIELD_HEIGHT;y++) {
		for (x=0;x<FIELD_WIDTH;x++) {
			if (field[x][y]==FIELD_EDGE) {
				printf("#");
			} else if (field[x][y]==GREEN_CAPTURE) {
				printf("G");
			} else if (field[x][y]==RED_CAPTURE) {
				printf("R");
			} else {
				printf(" ");
			}
		}
		printf("\r\n");
	}
	printf("\r\n");

	return;
}

void update(struct player *green, struct player *red)
{
	char buf[32];
	uint8_t x,y;

	// Draw next pixel
	x=green->x;
	y=green->y;
	Adafruit_ST7735_drawPixel(x,y,0x0000);
	x=red->x;
	y=red->y;
	Adafruit_ST7735_drawPixel(x,y,0xFFFF);

	// Update score
	graphics_fillRect(0,140,128,20,Adafruit_ST7735_Color565(128,128,128));
	graphics_setTextColor(0x0000);
	snprintf(buf,10,"%6u",green->score);
	graphics_drawText(buf,0,143);
	graphics_setTextColor(0xFFFF);
	snprintf(buf,10,"%6u",red->score);
	graphics_drawText(buf,64,143);
}

