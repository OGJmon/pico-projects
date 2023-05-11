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
#include "tft_stdout.h"

// Structure to hold functions that point to
// functions for IO through printf/scanf
stdio_driver_t stdio_tft = {
	.out_chars=tft_out_chars,	// stdout
	.out_flush=NULL,
	.in_chars=NULL,
	.next=NULL,
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
	.crlf_enabled = false,
#endif
};

// Write string to TFT
void tft_out_chars(const char *buf, int len)
{
	static char display[16][22]={0};	// Keep what's on the tft screen
	static uint32_t nl=false;			// Newline flag
	uint32_t i;							// Counter
	static uint32_t c=0;				// Current location on bottom line

	// if new line, move up all text
	if (nl==true) {
		for (i=0;i<15;i++) {
			memcpy(display[i],display[i+1],21);
		}
		memset(display[15],0,21);
		nl=false;	// Clear newline flag
		c=0;		// Move to first position
	}
	// add in new text
	for (i=0;i<len;i++) {
		// If we have embedded new line in string
		// Take care of each
		if (buf[i]=='\n') {
			// Scroll up if not at end of line
			if (i<len-1) {
				for (i=0;i<15;i++) {
					memcpy(display[i],display[i+1],21);
				}
				memset(display[15],0,21);
				nl=false;
			} else {
				// End of line
				nl=true;
			}
			c=0;
			// Do not add the newline to the tft print buffer
			continue;
		} else if (buf[i]=='\b') {	// Backspace	
			if (c>0) {	// Limits
				c--;
			}
			display[15][c]=0;	// Remove the "erased" character
			continue;
		}
		// Add text
		if (c<21) {
			display[15][c++]=buf[i];
		}
	}
	// Update display - we should move this over to DMA
	// to core 1
	Adafruit_ST7735_fillScreen(0x0000);
	for (i=0;i<16;i++) {
		graphics_drawText(display[i],0,i*10);
	}

	return;
}
