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
		void ApplyMoveOnBitboardSet(BitboardSet* board, Move* move);
		int EvaluatePosition(BitboardSet board, bool maximizingPlayer);
		int Minimax(BitboardSet board, int depth, bool colour, bool maximizingIsWhite, int takeOriginIndex);
	public:
		Bot(std::vector<Piece>* board_state);
		AppliedMove GenerateMove(std::vector<Piece>* board_state, int depth, bool isBotWhite,
			bool isCaptureChain, int forcedOriginX, int forcedOriginY);
};