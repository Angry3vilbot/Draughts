#pragma once
#include <vector>
#include "Piece.h"

struct ValidMove {
	int x;
	int y;
	bool isCapture;
};

std::vector<ValidMove> computeValidMoves(std::vector<Piece>& board_state, Piece& piece);
bool AnyPieceHasCaptures(std::vector<Piece>& board_state, bool colour);
bool PieceHasCaptures(std::vector<Piece>& board_state, Piece& piece);