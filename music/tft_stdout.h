// A. Sheaff 2023/01/05
// Function to direct printf to the TFT
#ifndef TFT_STDOUT_H
#define TFT_STDOUT_H

#include "pico/stdio/driver.h"

// Structure for the IO redirection
extern stdio_driver_t stdio_tft;

// stdout to TFT
void tft_out_chars(const char *buf, int len);

#endif	// TFT_STDOUT_H
