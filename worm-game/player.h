#ifndef PLAYER_H
#define PLAYER_H

#include "player.h"

struct player {
	uint8_t x;
	uint8_t y;
	int8_t move;
	uint8_t unused;
	uint32_t score;
};

void move(struct player *p, uint8_t *out_x, uint8_t *out_y);
void rules(struct player *green, struct player *red);

#endif	// PLAYER_H
