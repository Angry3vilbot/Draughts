#pragma once
#include "BitboardSet.h"
#include "Piece.h"
#include "AppliedMove.h"
#include "Move.h"
#include <vector>
#include "TTEntry.h"
#include "Zobrist.h"
// Evaluation weights
struct EvalWeights {
	int PIECE_FACTOR = 100,
		KING_FACTOR = 130,
		MOVER_COUNT_FACTOR = 5,
		PROMOTION_CANDIDATES_FACTOR = 10,
		DOUBLE_CORNER_FACTOR = 10;
};

class Bot {
	private:
		const size_t TT_SIZE = 1 << 24;
		const static int MAX_DEPTH = 32;
		const static int MIN_NULL_MOVE_DEPTH = 3;
		const static int MIN_NULL_MOVE_PIECE_COUNT = 8;
		const static int ASPIRATION_MARGIN = 30;
		const static int LMR_MIN_DEPTH = 3;
		const static int LMR_MIN_COUNT = 3;
		// White PST
		static constexpr int WHITE_PIECE_PST[32] = {
			0,  0,  0,  0,
			3,  3,  2,  2,
			6,  6,  6,  6,
			10, 10, 10, 10,
			16, 16, 16, 16,
			24, 24, 24, 24,
			30, 30, 30, 30,
			0,  0,  0,  0	// Row 8 cannot have non-king pieces standing on it, so it won't be evaluated
		};
		// Black PST
		static constexpr int BLACK_PIECE_PST[32] = {
			 0,  0,  0,  0,   // Same as with white pieces
			30, 30, 30, 30,
			24, 24, 24, 24,
			16, 16, 16, 16,
			10, 10, 10, 10,
			 6,  6,  6,  6,
			 3,  3,  3,  3,
			 0,  0,  0,  0
		};

		BitboardSet bitboards;
		Move killerMoves[MAX_DEPTH][2]{};
		std::vector<TTEntry> transpositionTable;
		EvalWeights weights;
		std::vector<uint64_t> searchPathHistory;
		void ResetTranspositionTable();
		void ResetKillerMoves();
		std::vector<Move> GenerateLegalMoves(const BitboardSet& bitboards, bool colour, unsigned int movers, unsigned int jumpers);
		void GenerateMovesFromSource(int source, bool isWhite, bool isKing,
		unsigned int WhitePieces, unsigned int BlackPieces, std::vector<Move>& result);
		void ApplyMoveOnBitboardSet(BitboardSet* board, Move* move);
		int EvaluatePosition(BitboardSet board, bool maximizingPlayer,
			unsigned int whiteMovers, unsigned int blackMovers, unsigned int whiteJumpers, unsigned int blackJumpers);
		int Minimax(BitboardSet board, int depth, bool colour, bool maximizingIsWhite, int takeOriginIndex,
			int alpha, int beta, bool isAfterNullMove, std::vector<uint64_t>& history);
	public:
		Bot(std::vector<Piece>* board_state);
		AppliedMove GenerateMove(std::vector<Piece>* board_state, int depth, bool isBotWhite,
			bool isCaptureChain, int forcedOriginX, int forcedOriginY, std::vector<uint64_t>& history, int& eval);
		void ResetBot(std::vector<Piece>* board_state);
};