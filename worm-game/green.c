#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "moves.h"
#include "field.h"
#include "debug.h"
#include "field_public.h"

// Red player appears black on the screen 
// Green player appears white on the screen 

// All functions use a single coordinate system with
//  an origin at the upper left corner of the TFT display
//  Pixel and field coordinate (0,0)

// To get what's at a pixel (cell) call the following function
//
// uint8_t field_get_cell(uint8_t x, uint8_t y)
//
//  where x is the x field coordinate and y is the y field coordinate
//  Returns a bitmap as follows:
//   All bits zero == uncaptured
//   bit 0 is one if (x,y) is Green captured
//   bit 1 is one if (x,y) is Red   captured
//   bit 2 is unused and zero
//   bit 3 is unused and zero
//   bit 4 is masked to zero
//   bit 5 is masked to zero
//   bit 6 is one if (x,y) is undefined location (out of bounds of field & edge)
//   bit 7 is one if (x,y) is on the edge of the field

// Interface to logic for the player
//  Player is passed the current location of us (us_x,us_y)
//   and the opponent's current location (them_x,them_y)
// Returns a move to the game engine.  Possible moves are relative is the
//  current location (us_x,us_y) is:
//  MOVE_NORTH (us_x,us_y-1)
//  MOVE_EAST  (us_x+1,us_y)
//  MOVE_SOUTH (us_x,us_y+1)
//  MOVE_WEST  (us_x-1,us_y)
//  MOVE_NONE  player skips turn
//  Any other return value is interpreted as MOVE_NONE
//  Invalid moves are silently discarded and interpreted as MOVE_NONE

// Play area is 128x140 pixels
//   with a 2 pixel wide edge border that is not playable
// Playable area is 124x136 pixels starting at (2,2)
// O indicates field and pixel origin at (0,0)
// E is the field edge and cannot be occupied by either player
// # indicates valid playable field location at (2,2)
// P indicates a valid playable field location

//    =====> +X direction
// |<================= 128 PIXELS =====================>|
// +-+-+-+-+-+------------------------------------+-+-+-+	-    ^
// |O|E|E|E|E|             ..........             |E|E|E|   ^    |
// |E|E|E|E|E|             ..........             |E|E|E|   |    N
// |E|E|#|P|P|             ..........             |P|E|E|   |    O        |
// |E|E|P|P|P|             ..........             |P|E|E|   |    R        |
// |E|E|P|P|P|             ..........             |P|E|E|        T        |
//      .                  ..........                .      1    H        v
//      .                  ..........                .      4
//      .                  ..........                .      0             +
//      .                  ..........                .                    Y
//      .                PLAYING FIELD IS            .      P    EAST ->
//      .                   124x136 PIXELS           .      I             d
//      .                  ..........                .      X  <-WEST     i
//      .                  ..........                .      E             r
//      .                  ..........                .      L             e
//      .                  ..........                .      S    S        c
//      .                  ..........                .           O        t
// |E|E|P|P|P|             ..........             |P|E|E|   |    U        i
// |E|E|P|P|P|             ..........             |P|E|E|   |    T        o
// |E|E|P|P|P|             ..........             |P|E|E|   |    H        n
// |E|E|E|E|E|             ..........             |E|E|E|   |    |
// |E|E|E|E|E|             ..........             |E|E|E|   v    v
// +-+-+-+-+-+------------------------------------+-+-+-+   -
// |         SCORE                   AREA               |
// +----------------------------------------------------+

// Below the field is the scoreboard.  The scoreboard area is out of bounds
// and not playable.

int8_t green_player(uint8_t us_x, uint8_t us_y, uint8_t them_x, uint8_t them_y)
{
	static int8_t m=MOVE_EAST;	// Our move

	return m;
}

// Add additional functions as needed
