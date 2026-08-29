#include "BitboardSet.h"
// Converts piece coordinates to the bit index of that square in the bitboard
int CoordinatesToBitIndex(int x, int y) {
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
