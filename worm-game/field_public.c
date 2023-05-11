#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/binary_info.h"
#include "hardware/structs/rosc.h"
#include "field_internal.h"

uint8_t field_get_cell(uint8_t x, uint8_t y)
{
	if (x>=FIELD_WIDTH) {
		return FIELD_UNDEF;
	}
	if (y>=FIELD_HEIGHT) {
		return FIELD_UNDEF;
	}

	return field[x][y];
}
