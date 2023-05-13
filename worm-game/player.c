#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "player.h"
#include "field_internal.h"
#include "moves.h"

void move(struct player *p, uint8_t *out_x, uint8_t *out_y)
{
	uint8_t x=p->x;
	uint8_t y=p->y;

	if (p->move==MOVE_WEST) {
		x--;
	} else if (p->move==MOVE_EAST) {
		x++;
	} else if (p->move==MOVE_NORTH) {
		y--;
	} else if (p->move==MOVE_SOUTH) {
		y++;
	}

	*out_x=x;
	*out_y=y;

	return;
}

void rules(struct player *green, struct player *red)
{
	// Take the moves and see if they player can actually move
	// Update the location
	// Increment score
	uint8_t gx, gy, rx, ry;

	// Calculate proposed locations
	move(green,&gx,&gy);
	move(red,&rx,&ry);
	// Propose to claim a location
	field[gx][gy]|=GREEN_PROPOSED;
	field[rx][ry]|=RED_PROPOSED;
	// If on edge discard proposed & reset to edge
	if ((field[gx][gy]&EDGE_MASK)==FIELD_EDGE) {
		field[gx][gy]=FIELD_EDGE;
	}
	if ((field[rx][ry]&EDGE_MASK)==FIELD_EDGE) {
		field[rx][ry]=FIELD_EDGE;
	}

	// Already captured - clear all proposed
	if ((field[gx][gy]&CAPTURE_MASK)!=UNCAPTURED) {
		field[gx][gy]&=CAPTURE_MASK;
	}
	if ((field[rx][ry]&CAPTURE_MASK)!=UNCAPTURED) {
		field[rx][ry]&=CAPTURE_MASK;
	}
	// Both try to claim gx==rx && gy==ry - No one gets - clear all
	if (field[gx][gy]==(GREEN_PROPOSED|RED_PROPOSED)) {
		field[gx][gy]=UNCAPTURED;
	}
	// Previous should clear this
	if (field[rx][ry]==(GREEN_PROPOSED|RED_PROPOSED)) {
		field[rx][ry]=UNCAPTURED;
	}
	// Only one player now has a proposed claim with no one else claiming
	if (field[gx][gy]==GREEN_PROPOSED) {
		field[gx][gy]=GREEN_CAPTURE;
		green->x=gx;
		green->y=gy;
		green->score++;
	}
	if (field[rx][ry]==RED_PROPOSED) {
		field[rx][ry]=RED_CAPTURE;
		red->x=rx;
		red->y=ry;
		red->score++;
	}
	return;
}
