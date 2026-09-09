#include "Zobrist.h"
#include <random>


uint64_t pieceTable[32][4];
uint64_t chainOriginTable[32];
uint64_t sideToMoveHash;
// Initialize the Zobrist hash by generating random values for the tables and the side to move
void InitZobristHash() {
	// Initialize the 64-bit Mersenne Twister and the uniform integer distribution
	std::mt19937_64 rng(12345);
	std::uniform_int_distribution<uint64_t> dist;

	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 4; j++) {
			// Init the specified piece type hash for the specified square
			pieceTable[i][j] = dist(rng);
		}
		// Init the chain origin table
		chainOriginTable[i] = dist(rng);
	}
	// Init the side to move hash
	sideToMoveHash = dist(rng);
}
// Compute the Zobrist Hash for the position
uint64_t ComputeZobristHash(const BitboardSet& board, bool colour, int takeOriginIndex) {
	uint64_t hash = 0;
	// If it's white player's turn to move, XOR in the sideToMoveHash.
	if (colour) hash ^= sideToMoveHash;
	// If there is a take origin index (i.e. this is a capture chain)
	// XOR in the appropriate hash in the chain origin table
	if (takeOriginIndex != -1) hash ^= chainOriginTable[takeOriginIndex];
	// XOR in every piece on the board
	for (int i = 0; i < 32; i++) {
		// Mask to find the piece (or lack thereof) in the bitboard
		unsigned int squareMask = 1u << i;
		if (board.WhitePieces & squareMask) {
			bool isKing = board.Kings & squareMask;
			// XOR in the piece
			hash ^= pieceTable[i][isKing ? whiteKing : whitePiece];
		}
		else if (board.BlackPieces & squareMask) {
			bool isKing = board.Kings & squareMask;
			// XOR in the piece
			hash ^= pieceTable[i][isKing ? blackKing : blackPiece];
		}
	}

	return hash;
}