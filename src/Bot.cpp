#include "Bot.h"
#include <bit>
#include <iostream>

Bot::Bot(std::vector<Piece>* board_state) : bitboards(board_state) {}
// Extract every set bit from a bitboard into a vector as an index
std::vector<int> Bot::GetSetBits(unsigned int bits) {
	std::vector<int> result;
	while (bits) {
		// Count trailing zeros to find the index of the lowest set bit
		int index = std::countr_zero(bits);
		result.push_back(index);
		// Clear the lowest set bit
		bits &= bits - 1;
	}
	return result;
}
// Converts the bit index of a square in the bitboard to piece coordinates
void BitIndexToCoordinates(int index, int* resultX, int* resultY) {
	int row = index / 4; // zero-indexed
	int col = index % 4; // zero-indexed
	*resultY = row + 1;
	*resultX = row % 2 == 0 ? col * 2 + 1 : col * 2 + 2;
}
// Generate every Move for a source square
std::vector<Move> Bot::GenerateMovesFromSource(int source, bool isWhite, bool isKing, unsigned int WhitePieces, unsigned int BlackPieces)
{
	unsigned int occupied = WhitePieces | BlackPieces;
	unsigned int notOccupied = ~occupied;
	unsigned int opponent = isWhite ? BlackPieces : WhitePieces;
	unsigned int sourceBoard = 1u << source;
	std::vector<Move> result;
	result.reserve(4);

	// Helper lambda function. Tries to add a move to result if it's valid.
	// delta is the change to the index used to calculate the destination using the source.
	auto tryRegisterMove = [&](int delta) {
		int destination = source + delta;
		if (destination < 0 || destination > 31) return;

		// If the destination is unoccupied
		if ((notOccupied >> destination) & 1) {
			bool isCrown = false;
			if (!isKing) {
				// If the piece is not a king and is on the opponent's back row, the move crowns the piece
				if ((isWhite && destination >= 28) || (!isWhite && destination <= 3)) isCrown = true;
			}
			result.push_back({source, destination, false, -1, isCrown});
		}
		// If the destination is an opponent
		else if ((opponent >> destination) & 1) {
			// The actual destination is one tile past the opponent
			// Coordinates of the source square, the opponent and the actual destination square
			int sourceX, sourceY, opponentX, opponentY, destinationX, destinationY;
			BitIndexToCoordinates(source, &sourceX, &sourceY);
			BitIndexToCoordinates(destination, &opponentX, &opponentY);
			// Delta of the move, to find the square after the opponent on the same diagonal
			int deltaX = opponentX - sourceX;
			int deltaY = opponentY - sourceY;
			destinationX = opponentX + deltaX;
			destinationY = opponentY + deltaY;
			if (destinationX < 1 || destinationX > 8 || destinationY < 1 || destinationY > 8) return;

			int finalDestination = bitboards.CoordinatesToBitIndex(destinationX, destinationY);
			if (finalDestination < 32 && finalDestination >= 0 && (notOccupied >> finalDestination) & 1) {
				bool isCrown = false;
				if (!isKing) {
					// If the piece is not a king and is on the opponent's back row, the move crowns the piece
					if ((isWhite && finalDestination >= 28) || (!isWhite && finalDestination <= 3)) isCrown = true;
				}
				result.push_back({ source, finalDestination, true, destination, isCrown });
			}
		}
	};
	
	// Moving forward (up for White, down for Black, both for kings)
	if (isWhite || isKing) {
		// Shift by 4 is always legal, as long as it doesn't go past 31
		tryRegisterMove(4);
		if (sourceBoard & MASK_L3) tryRegisterMove(3);
		if (sourceBoard & MASK_L5) tryRegisterMove(5);
	}
	if (!isWhite || isKing) {
		// Shift by 4 is always legal, as long as it doesn't go below 0
		tryRegisterMove(-4);
		if (sourceBoard & MASK_R3) tryRegisterMove(-3);
		if (sourceBoard & MASK_R5) tryRegisterMove(-5);
	}
	return result;
}
// Generate all legal moves for the chosen player in the chosen bitboard set
std::vector<Move> Bot::GenerateLegalMoves(BitboardSet bitboards, bool colour) {
	std::vector<Move> result;
	unsigned int jumpersBitboard = colour ? bitboards.GetJumpersWhite() : bitboards.GetJumpersBlack();
	// The bitboard that is actually going to be used.
	// If there are any jumpers it's going to be the jumpers bitboard.
	// If there are none, it's going to be the movers.
	unsigned int sourceBitboard = jumpersBitboard ? jumpersBitboard : 
		(colour ? bitboards.GetMoversWhite() : bitboards.GetMoversBlack());
	bool capturesOnly = jumpersBitboard != 0;

	for (int source : GetSetBits(sourceBitboard)) {
		bool isKing = (bitboards.Kings >> source) & 1;
		std::vector<Move> pieceMoves = 
			GenerateMovesFromSource(source, colour, isKing, bitboards.WhitePieces, bitboards.BlackPieces);
		for (Move& move : pieceMoves) {
			if (!capturesOnly || move.isCapture) result.push_back(move);
		}
	}
	return result;
}
// Apply the move to a copy of the board for minimax
void Bot::ApplyMoveOnBitboardSet(BitboardSet* board, Move* move) {
	unsigned int removalMask = ~(1u << move->from);
	unsigned int additionMask = 1u << move->to;
	bool isWhite = (board->WhitePieces >> move->from) & 1;
	bool isKing = (board->Kings >> move->from) & 1;
	// Remove the piece from its original position on the colour bitboard
	// and add it to the destination
	if (isWhite) board->WhitePieces = (board->WhitePieces & removalMask) | additionMask;
	else board->BlackPieces = (board->BlackPieces & removalMask) | additionMask;
	// If the piece is a king, remove it from its original position on the kings bitboard
	if (isKing) board->Kings = board->Kings & removalMask;
	// If the piece is a king, or is crowned, add it to the destination
	if (isKing || move->isCrown) board->Kings = board->Kings | additionMask;
	// If the move is a capture, remove the taken piece from the opposite colour bitboard
	if (move->isCapture) {
		removalMask = ~(1u << move->capturedSquare);

		if (isWhite) board->BlackPieces = board->BlackPieces & removalMask;
		else board->WhitePieces = board->WhitePieces & removalMask;
		board->Kings = board->Kings & removalMask;
	}
}
// Evaluates the current position on the provided bitboards
int Bot::EvaluatePosition(BitboardSet board, bool maximizingPlayer) {
	int result = 0;
	unsigned int whiteMovers = board.GetMoversWhite();
	unsigned int blackMovers = board.GetMoversBlack();
	unsigned int whiteJumpers = board.GetJumpersWhite();
	unsigned int blackJumpers = board.GetJumpersBlack();
	// Evaluation weights
	const int PIECE_FACTOR = 1, KING_FACTOR = 3, MOVER_COUNT_FACTOR = 1, PROMOTION_CANDIDATES_FACTOR = 3, DOUBLE_CORNER_FACTOR = 5;
	// Counters for all stats being evaluated
	int white_pieces = std::popcount(board.WhitePieces);
	int	black_pieces = std::popcount(board.BlackPieces);
	int white_kings = std::popcount(board.WhitePieces & board.Kings);
	int	black_kings = std::popcount(board.BlackPieces & board.Kings);
	int white_movers = std::popcount(whiteMovers | whiteJumpers);
	int black_movers = std::popcount(blackMovers | blackJumpers);
	int white_promotion_candidates = std::popcount(
		((board.WhitePieces & ~board.Kings) & MASK_ROW7 & whiteMovers)
		| ((board.WhitePieces & ~board.Kings) & MASK_ROW6 & whiteJumpers));
	int black_promotion_candidates = std::popcount(
		((board.BlackPieces & ~board.Kings) & MASK_ROW2 & blackMovers)
		| ((board.BlackPieces & ~board.Kings) & MASK_ROW3 & blackJumpers));
	int white_double_corner_kings = std::popcount(board.WhitePieces & board.Kings & MASK_DCORNER);
	int black_double_corner_kings = std::popcount(board.BlackPieces & board.Kings & MASK_DCORNER);

	// Evaluate the position
	result += maximizingPlayer ? (white_pieces - black_pieces) * PIECE_FACTOR : (black_pieces - white_pieces) * PIECE_FACTOR;
	result += maximizingPlayer ? (white_kings - black_kings) * KING_FACTOR : (black_kings - white_kings) * KING_FACTOR;
	result += maximizingPlayer ? (white_movers - black_movers) * MOVER_COUNT_FACTOR : (black_movers - white_movers) * MOVER_COUNT_FACTOR;
	result += maximizingPlayer ? (white_promotion_candidates - black_promotion_candidates) * PROMOTION_CANDIDATES_FACTOR
		: (black_promotion_candidates - white_promotion_candidates) * PROMOTION_CANDIDATES_FACTOR;
	result += maximizingPlayer ? (white_double_corner_kings - black_double_corner_kings) * DOUBLE_CORNER_FACTOR
		: (black_double_corner_kings - white_double_corner_kings) * DOUBLE_CORNER_FACTOR;
	return result;
}
// Runs the minimax algorithm for the current depth and player/board state
int Bot::Minimax(BitboardSet board, int depth, bool colour, bool maximizingIsWhite, int takeOriginIndex) {
	// If the bot ran out of depth, finish the branch
	if (depth == 0) return EvaluatePosition(board, maximizingIsWhite);

	std::vector<Move> legalMoves = GenerateLegalMoves(board, colour);
	bool isMaximizing = colour == maximizingIsWhite;
	int score;
	// If there are no legal moves, the maximizing player lost
	if (legalMoves.empty()) return isMaximizing ? INT_MIN : INT_MAX;

	score = isMaximizing ? INT_MIN : INT_MAX;
	bool takesOnly = legalMoves[0].isCapture;
	// If the move is not part of a take chain, perform standard minimax
	// Otherwise, if takes are available, only consider moves made by the piece that started the chain
	for (Move& move : legalMoves) {
		if (takeOriginIndex != -1 && takesOnly && takeOriginIndex != move.from) continue;
		// Make a copy of the current board and apply the move to it
		BitboardSet nextBoard = board;
		ApplyMoveOnBitboardSet(&nextBoard, &move);
		// Run minimax for the next move with the new position
		int eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1);
		score = isMaximizing ? std::max(score, eval) : std::min(score, eval);
	}

	return score;
}

// Generate a move using the Minimax rule to find the best possible move for the bot with the given depth
AppliedMove Bot::GenerateMove(std::vector<Piece>* board_state, int depth,
	bool botColour, bool isCaptureChain, int forcedOriginX, int forcedOriginY) {
	AppliedMove result = { -1, -1, -1, -1, -1, -1, 0, 0 };
	Move chosenMove{};
	int highScore = INT_MIN;
	// Update the bitboards with the current board state
	bitboards.UpdateBitboards(board_state);
	// Generate all of the possible legal moves for the current player (the bot)
	std::vector<Move> legalMoves = GenerateLegalMoves(bitboards, botColour);
	// If it's a capture chain, filter the legal moves if the originator piece has moves available
	if (isCaptureChain) {
		int forcedOriginIndex = bitboards.CoordinatesToBitIndex(forcedOriginX, forcedOriginY);
		bool originHasMove = false;
		// Figure out if the chain originator has any moves (takes) available
		for (const Move& move : legalMoves) {
			if (move.from == forcedOriginIndex) originHasMove = true;
		}
		// If it does, then filter the moves
		if (originHasMove) {
			std::erase_if(legalMoves, [&](const Move& m) { return m.from != forcedOriginIndex; });
		}
	}
	// If there are no legal moves, the game is over
	if (legalMoves.empty()) {
		std::cout << std::endl;
		std::cout << "The bot has lost. The game is over." << std::endl;
		return result;
	}
	for (const Move& move : legalMoves) {
		BitboardSet nextBoard = bitboards;
		Move moveCopy = move;
		ApplyMoveOnBitboardSet(&nextBoard, &moveCopy);

		int score = Minimax(nextBoard, depth - 1, !botColour, botColour, moveCopy.isCapture ? moveCopy.to : -1);
		chosenMove = score >= highScore ? move : chosenMove;
		highScore = score >= highScore ? score : highScore;
	}

	int sourceX, sourceY, destinationX, destinationY, captureX, captureY;
	BitIndexToCoordinates(chosenMove.from, &sourceX, &sourceY);
	BitIndexToCoordinates(chosenMove.to, &destinationX, &destinationY);
	if(chosenMove.isCapture) BitIndexToCoordinates(chosenMove.capturedSquare, &captureX, &captureY);

	result = {
		sourceX, sourceY,
		destinationX, destinationY,
		chosenMove.isCapture ? captureX : -1,
		chosenMove.isCapture ? captureY : -1,
		chosenMove.isCapture,
		chosenMove.isCrown
	};
	return result;
}