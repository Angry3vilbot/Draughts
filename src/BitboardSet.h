#pragma once
#include <vector>
#include "Piece.h"

class BitboardSet {
	public:
		unsigned int WhitePieces = 0;
		unsigned int BlackPieces = 0;
		unsigned int Kings = 0;

		BitboardSet(std::vector<Piece>* board_state);
		void UpdateBitboards(std::vector<Piece>* board_state);
};