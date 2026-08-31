#pragma once
// Used for the bot's minimax tree
struct Move {
	int from, to; // bit index of the locations on the bitboard
	bool isCapture;
	int capturedSquare; // bit index of the piece on the bitboard
	bool isCrown;
};