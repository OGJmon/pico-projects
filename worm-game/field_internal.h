#ifndef FIELD_INTERNAL_H
#define FIELD_INTERNAL_H

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "player.h"
#include "field.h"

extern uint8_t field[FIELD_WIDTH][FIELD_HEIGHT];

void field_print(void);
void update(struct player *green, struct player *red);

#endif // FIELD_INTERNAL_H
