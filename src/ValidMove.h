#pragma once
#include <vector>
#include "Piece.h"

struct ValidMove {
	int x;
	int y;
	bool isCapture;
};

std::vector<ValidMove> computeValidMoves(std::vector<Piece>& board_state, Piece& piece);