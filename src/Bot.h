#pragma once
#include "BitboardSet.h"
#include "Piece.h"
#include "AppliedMove.h"
#include "Move.h"
#include <vector>
#include "TTEntry.h"
#include "Zobrist.h"
class Bot {
	private:
		const size_t TT_SIZE = 1 << 22;
		BitboardSet bitboards;
		std::vector<TTEntry> transpositionTable;
		void ResetTranspositionTable();
		std::vector<Move> GenerateLegalMoves(const BitboardSet& bitboards, bool colour, unsigned int movers, unsigned int jumpers);
		void GenerateMovesFromSource(int source, bool isWhite, bool isKing,
		unsigned int WhitePieces, unsigned int BlackPieces, std::vector<Move>& result);
		void ApplyMoveOnBitboardSet(BitboardSet* board, Move* move);
		int EvaluatePosition(BitboardSet board, bool maximizingPlayer,
			unsigned int whiteMovers, unsigned int blackMovers, unsigned int whiteJumpers, unsigned int blackJumpers);
		int Minimax(BitboardSet board, int depth, bool colour, bool maximizingIsWhite, int takeOriginIndex, int alpha, int beta);
	public:
		Bot(std::vector<Piece>* board_state);
		AppliedMove GenerateMove(std::vector<Piece>* board_state, int depth, bool isBotWhite,
			bool isCaptureChain, int forcedOriginX, int forcedOriginY);
};