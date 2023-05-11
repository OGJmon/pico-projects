#ifndef FIELD_H
#define FIELD_H

// Size of playing field in pixels
#define FIELD_WIDTH 128
#define FIELD_HEIGHT 140

// Field bitmap
// Bits:	7	Edge
//			6	Undefined location
//			5	Red Proposed
//			4	Green Proposed
//			3	Unused
//			2	Unused
//			1	Red Capture
//			0	Green Capture
#define UNCAPTURED 0x00

#define GREEN_CAPTURE 0x01
#define RED_CAPTURE 0x02

#define GREEN_PROPOSED 0x10
#define RED_PROPOSED 0x20

#define FIELD_EDGE 0x80
#define FIELD_UNDEF 0x40

#define CAPTURE_MASK 0x03
#define PROPOSE_MASK 0x30
#define EDGE_MASK 0x80
#define UNDEF_MASK 0x40

#endif // FIELD_H
