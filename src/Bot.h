#pragma once
#include "BitboardSet.h"
#include "Piece.h"
#include "AppliedMove.h"
#include "Move.h"
#include <vector>
class Bot {
	private:
		BitboardSet bitboards;
		std::vector<Move> GenerateLegalMoves(BitboardSet bitboards, bool colour);
		std::vector<Move> GenerateMovesFromSource
			(int source, bool isWhite, bool isKing, unsigned int WhitePieces, unsigned int BlackPieces);
		std::vector<int> GetSetBits(unsigned int bits);
	public:
		Bot(std::vector<Piece>* board_state);
		void GenerateMove(std::vector<Piece>* board_state, int depth, bool isBotWhite);
};