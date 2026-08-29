#pragma once
#include "BitboardSet.h"
#include "BitboardMasks.h"
#include "Piece.h"
#include "AppliedMove.h"
#include <vector>
class Bot {
	private:
		BitboardSet bitboards;
		// insert helper methods here
	public:
		Bot(std::vector<Piece>* board_state);
		AppliedMove GenerateMove(std::vector<Piece>* board_state, int depth);
};