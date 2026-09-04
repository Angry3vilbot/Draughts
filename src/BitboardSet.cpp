#include "BitboardSet.h"
// Converts piece coordinates to the bit index of that square in the bitboard
int BitboardSet::CoordinatesToBitIndex(int x, int y) {
	// On the board, every *odd* row has the *odd* columns in play
	// while every *even* row has the *even* columns in play
	int row = y - 1; // zero-indexed
	x = x - 1; // zero-indexed
	int col = x / 2; // 0,2,4,6 => 0,1,2,3; 1,3,5,7 => 0,1,2,3
	return row * 4 + col;
}

BitboardSet::BitboardSet(std::vector<Piece>* board_state) {
	UpdateBitboards(board_state);
}

void BitboardSet::UpdateBitboards(std::vector<Piece>* board_state) {
	WhitePieces = 0;
	BlackPieces = 0;
	Kings = 0;

	for (Piece piece : *board_state) {
		int bitIndex = CoordinatesToBitIndex(piece.getX(), piece.getY());
		if (piece.getIsWhite()) {
			// Set the bit at bitIndex to 1 in the WhitePieces bitboard
			WhitePieces = WhitePieces | (1u << bitIndex);
		}
		else {
			// Set the bit at bitIndex to 1 in the BlackPieces bitboard
			BlackPieces = BlackPieces | (1u << bitIndex);
		}
		// Set the bit at bitIndex to 1 in the Kings bitboard if the piece is a king
		if (piece.getIsKing()) Kings = Kings | (1u << bitIndex);
	}
}
// Get all White pieces that can move (excluding jumping)
unsigned int BitboardSet::GetMoversWhite() {
	// Unoccupied squares
	unsigned int notOccupied = ~(WhitePieces | BlackPieces);
	// White Kings
	unsigned int whiteKings = WhitePieces & Kings;
	// White pieces that can move. Right shifting an unoccupied space by 4 to find the piece that can go there, if there is one.
	unsigned int movers = (notOccupied >> 4) & WhitePieces;
	// Logical OR to include the L3 mask. Shifted RIGHT to find the starting square.
	movers |= ((notOccupied & MASK_R3) >> 3) & WhitePieces;
	// Logical OR to include the L5 mask. Shifted RIGHT to find the starting square.
	movers |= ((notOccupied & MASK_R5) >> 5) & WhitePieces;
	if (whiteKings) {
		// Find backwards squares for moving kings
		// Logical OR to include the guaranteed right shift by 4. Shifted LEFT to find the starting square.
		movers |= (notOccupied << 4) & whiteKings;
		// Logical OR to include the R3 mask. Shifted LEFT to find the starting square.
		movers |= ((notOccupied & MASK_L3) << 3) & whiteKings;
		// Logical OR to include the R5 mask. Shifted LEFT to find the starting square.
		movers |= ((notOccupied & MASK_L5) << 5) & whiteKings;
	}
	return movers;
}
// Get all Black pieces that can move (excluding jumping)
unsigned int BitboardSet::GetMoversBlack() {
	// Unoccupied squares
	unsigned int notOccupied = ~(WhitePieces | BlackPieces);
	// Black Kings
	unsigned int blackKings = BlackPieces & Kings;
	// Black pieces that can move. Left shifting an unoccupied space by 4 to find the piece that can go there, if there is one.
	unsigned int movers = (notOccupied << 4) & BlackPieces;
	// Logical OR to include the L3 mask. Shifted LEFT to find the starting square.
	movers |= ((notOccupied & MASK_L3) << 3) & BlackPieces;
	// Logical OR to include the L5 mask. Shifted LEFT to find the starting square.
	movers |= ((notOccupied & MASK_L5) << 5) & BlackPieces;
	if (blackKings) {
		// Find backwards squares for moving kings
		// Logical OR to include the guaranteed left shift by 4. Shifted RIGHT to find the starting square.
		movers |= (notOccupied >> 4) & blackKings;
		// Logical OR to include the R3 mask. Shifted RIGHT to find the starting square.
		movers |= ((notOccupied & MASK_R3) >> 3) & blackKings;
		// Logical OR to include the R5 mask. Shifted RIGHT to find the starting square.
		movers |= ((notOccupied & MASK_R5) >> 5) & blackKings;
	}
	return movers;
}
// Get all White pieces that can jump (i.e. take)
unsigned int BitboardSet::GetJumpersWhite() {
	// Unoccupied squares
	unsigned int notOccupied = ~(WhitePieces | BlackPieces);
	// White Kings
	unsigned int whiteKings = WhitePieces & Kings;
	// White pieces that can jump
	unsigned int jumpers = 0;
	// Black pieces that are diagonally adjacent to an empty square that is currently being considered
	unsigned int currentPossibleVictims = (notOccupied >> 4) & BlackPieces;
	if (currentPossibleVictims) {
		// White pieces that are diagonally adjacent to the black pieces that are currently being considered
		jumpers |= (((currentPossibleVictims & MASK_R3) >> 3) | ((currentPossibleVictims & MASK_R5) >> 5)) & WhitePieces;
	}
	// Check the other unoccupied square direction
	currentPossibleVictims = ( ((notOccupied & MASK_R3) >> 3) | ((notOccupied & MASK_R5) >> 5) ) & BlackPieces;
	jumpers |= (currentPossibleVictims >> 4) & WhitePieces;
	if (whiteKings) {
		// Ditto, but backwards
		currentPossibleVictims = (notOccupied << 4) & BlackPieces;
		if (currentPossibleVictims) {
			jumpers |= (((currentPossibleVictims & MASK_L3) << 3) | ((currentPossibleVictims & MASK_L5) << 5)) & whiteKings;
		}

		currentPossibleVictims = (((notOccupied & MASK_L3) << 3) | ((notOccupied & MASK_L5) << 5)) & BlackPieces;
		if (currentPossibleVictims) {
			jumpers |= (currentPossibleVictims << 4) & whiteKings;
		}
	}

	return jumpers;
}
// Get all Black pieces that can jump (i.e. take)
unsigned int BitboardSet::GetJumpersBlack() {
	// Unoccupied squares
	unsigned int notOccupied = ~(WhitePieces | BlackPieces);
	// Black Kings
	unsigned int blackKings = BlackPieces & Kings;
	// Black pieces that can jump
	unsigned int jumpers = 0;
	// White pieces that are diagonally adjacent to an empty square that is currently being considered
	unsigned int currentPossibleVictims = (notOccupied << 4) & WhitePieces;
	if (currentPossibleVictims) {
		// Black pieces that are diagonally adjacent to the white pieces that are currently being considered
		jumpers |= (((currentPossibleVictims & MASK_L3) << 3) | ((currentPossibleVictims & MASK_L5) << 5)) & BlackPieces;
	}
	// Check the other unoccupied square direction
	currentPossibleVictims = ( ((notOccupied & MASK_L3) << 3) | ((notOccupied & MASK_L5) << 5) ) & WhitePieces;
	jumpers |= (currentPossibleVictims << 4) & BlackPieces;
	if (blackKings) {
		// Ditto, but backwards
		currentPossibleVictims = (notOccupied >> 4) & WhitePieces;
		if (currentPossibleVictims) {
			jumpers |= (((currentPossibleVictims & MASK_R3) >> 3) | ((currentPossibleVictims & MASK_R5) >> 5)) & blackKings;
		}

		currentPossibleVictims = (((notOccupied & MASK_R3) >> 3) | ((notOccupied & MASK_R5) >> 5)) & WhitePieces;
		if (currentPossibleVictims) {
			jumpers |= (currentPossibleVictims >> 4) & blackKings;
		}
	}

	return jumpers;
}