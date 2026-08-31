#pragma once
#include <vector>
#include "Piece.h"
#include "BitboardMasks.h"

class BitboardSet {
	public:
		unsigned int WhitePieces = 0;
		unsigned int BlackPieces = 0;
		unsigned int Kings = 0;

		BitboardSet(std::vector<Piece>* board_state);
		void UpdateBitboards(std::vector<Piece>* board_state);
		unsigned int GetMoversWhite();
		unsigned int GetMoversBlack();
		unsigned int GetJumpersWhite();
		unsigned int GetJumpersBlack();
		int CoordinatesToBitIndex(int x, int y);
};