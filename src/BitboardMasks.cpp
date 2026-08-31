#include "BitboardMasks.h"
// The playable tiles
unsigned int squares[32];
// Left-Shift and Right-Shift masks
unsigned int MASK_L5, MASK_L3, MASK_R5, MASK_R3;

void InitBitboardMasks() {
	squares[0] = 1;
	// Initialize every square as a separate bitmask
	for (int i = 1; i < 32; i++) squares[i] = 1 << i;
	// Initialize every shift mask. Each mask has bits set to 1
	// corresponding to every square on which this shift will lead to a valid position.
	// For example: MASK_L3 is a bitmask of squares where doing a left-shift by 3 will give you a valid square.
	// This is nesessary because odd and even rows are shifted slightly, so while a shift by 4 will always lead to a valid square,
	// that is not the case with finding the opposite square.
	MASK_L3 = squares[1] | squares[2] | squares[3] | squares[9] | squares[10] | squares[11] | squares[17] | squares[18] | squares[19] | squares[25] | squares[26] | squares[27];
	MASK_L5 = squares[4] | squares[5] | squares[6] | squares[12] | squares[13] | squares[14] | squares[20] | squares[21] | squares[22];
	MASK_R3 = squares[28] | squares[29] | squares[30] | squares[20] | squares[21] | squares[22] | squares[12] | squares[13] | squares[14] | squares[4] | squares[5] | squares[6];
	MASK_R5 = squares[25] | squares[26] | squares[27] | squares[17] | squares[18] | squares[19] | squares[9] | squares[10] | squares[11];
}