#include "Bot.h"
#include <bit>
#include <iostream>
#include <algorithm>

Bot::Bot(std::vector<Piece>* board_state) : bitboards(board_state), transpositionTable(TT_SIZE), NodeCounter(0) {}
// Clear the transposition table, done before a new game
void Bot::ResetTranspositionTable() {
	std::fill(transpositionTable.begin(), transpositionTable.end(), TTEntry{});
}
// Reset the killer moves array
void Bot::ResetKillerMoves() {
	Move reset{-1, -1, false, -1, false};
	for (int i = 0; i < MAX_DEPTH; i++) {
		killerMoves[i][0] = reset;
		killerMoves[i][1] = reset;
	}
}
// Converts the bit index of a square in the bitboard to piece coordinates
void BitIndexToCoordinates(int index, int* resultX, int* resultY) {
	int row = index / 4; // zero-indexed
	int col = index % 4; // zero-indexed
	*resultY = row + 1;
	*resultX = row % 2 == 0 ? col * 2 + 1 : col * 2 + 2;
}
// Generate every Move for a source square
void Bot::GenerateMovesFromSource(int source, bool isWhite, bool isKing,
	unsigned int WhitePieces, unsigned int BlackPieces, std::vector<Move>& result)
{
	unsigned int occupied = WhitePieces | BlackPieces;
	unsigned int notOccupied = ~occupied;
	unsigned int opponent = isWhite ? BlackPieces : WhitePieces;
	unsigned int sourceBoard = 1u << source;
	

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
}
// Generate all legal moves for the chosen player in the chosen bitboard set
std::vector<Move> Bot::GenerateLegalMoves(const BitboardSet& bitboards, bool colour,
	unsigned int movers, unsigned int jumpers) {
	std::vector<Move> result;
	result.reserve(12);
	// The bitboard that is actually going to be used.
	// If there are any jumpers it's going to be the jumpers bitboard.
	// If there are none, it's going to be the movers.
	unsigned int sourceBitboard = jumpers ? jumpers : movers;
	bool capturesOnly = jumpers != 0;
	std::vector<Move> pieceMoves;
	pieceMoves.reserve(4);
	// Generate moves for every piece
	unsigned int bits = sourceBitboard;
	while (bits) {
		// Count trailing zeros to find the index of the lowest set bit
		int sourceIndex = std::countr_zero(bits);
		// Clear the lowest set bit
		bits &= bits - 1;
		bool isKing = (bitboards.Kings >> sourceIndex) & 1;
		pieceMoves.clear();

		GenerateMovesFromSource(sourceIndex, colour, isKing, bitboards.WhitePieces, bitboards.BlackPieces, pieceMoves);
		for (Move& move : pieceMoves) {
			if (!capturesOnly || move.isCapture) result.push_back(move);
		}
	}
	// Sort the moves heuristically
	std::sort(result.begin(), result.end(), [colour](const Move& a, const Move& b) {
		// Prioritize moves that crown a piece
		if (a.isCrown != b.isCrown) return a.isCrown > b.isCrown;
		// Discourage opening the back row
		bool aBackRow = colour ? a.from <= 3 : a.from >= 28;
		bool bBackRow = colour ? b.from <= 3 : b.from >= 28;
		if (aBackRow != bBackRow) return !aBackRow;

		return false;
	});

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
int Bot::EvaluatePosition(BitboardSet board, bool maximizingPlayer,
	unsigned int whiteMovers, unsigned int blackMovers, unsigned int whiteJumpers, unsigned int blackJumpers) {
	int result = 0;
	// Evaluation weights
	const int PIECE_FACTOR = 1, KING_FACTOR = 3, MOVER_COUNT_FACTOR = 1, PROMOTION_CANDIDATES_FACTOR = 2, DOUBLE_CORNER_FACTOR = 5;
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
int Bot::Minimax(BitboardSet board, int depth, bool colour, bool maximizingIsWhite, int takeOriginIndex, 
	int alpha, int beta, bool isAfterNullMove) {
	NodeCounter++;

	// If the bot ran out of depth, finish the branch
	if (depth == 0) {
		unsigned int whiteMovers = board.GetMoversWhite();
		unsigned int blackMovers = board.GetMoversBlack();
		unsigned int whiteJumpers = board.GetJumpersWhite();
		unsigned int blackJumpers = board.GetJumpersBlack();
		return EvaluatePosition(board, maximizingIsWhite, whiteMovers, blackMovers, whiteJumpers, blackJumpers);
	}
	unsigned movers = colour ? board.GetMoversWhite() : board.GetMoversBlack();
	unsigned jumpers = colour ? board.GetJumpersWhite() : board.GetJumpersBlack();
	int originalAlpha = alpha;
	int originalBeta = beta;
	// If the depth is above 0, hash the board
	uint64_t hash = ComputeZobristHash(board, colour, takeOriginIndex);
	size_t index = hash & (TT_SIZE - 1);
	// Check if there is a hit in the transposition table for the index
	TTEntry& entry = transpositionTable[index];
	if (entry.depth >= depth && entry.hash == hash) {
		// There is a hit, check the type of score the entry has
		switch (entry.bound) {
		case 0:
			// The score is exact. We know the eval of the position and can return early.
			return entry.score;
		case 1:
			// The score is the upper bound. We can lower the beta if it is higher than the score
			if (beta > entry.score) {
				beta = entry.score;
				// Check if we can cut off the branch early with the updated beta
				if (alpha >= beta) {
					return entry.score;
				}
			}
			break;
		case -1:
			// The score is the lower bound. We can increase the alpha if it is lower than the score
			if (alpha < entry.score) {
				alpha = entry.score;
				// Check if we can cut off the branch early with the updated alpha
				if (alpha >= beta) {
					return entry.score;
				}
			}
		}
	}
	// Check if it is possible to do a run of Null-move pruning
	if (!jumpers && !isAfterNullMove && takeOriginIndex == -1 && depth >= MIN_NULL_MOVE_DEPTH) {
		int r = 2; // Reduction
		int nullEval = Minimax(board, depth - 1 - r, !colour, maximizingIsWhite, takeOriginIndex, alpha, beta, true);
		// Check if we can cut off early with the null evaluation
		bool isMaximizing = colour == maximizingIsWhite;
		if (isMaximizing && nullEval >= beta) return nullEval;
		if (!isMaximizing && nullEval <= alpha) return nullEval;
	}

	std::vector<Move> legalMoves = GenerateLegalMoves(board, colour, movers, jumpers);
	bool isMaximizing = colour == maximizingIsWhite;
	int score;

	// If it's a capture chain, filter the legal moves if the originator piece has moves available
	if (takeOriginIndex != -1) {
		bool originHasMove = false;
		// Figure out if the chain originator has any moves (takes) available
		for (const Move& move : legalMoves) {
			if (move.from == takeOriginIndex) originHasMove = true;
		}
		// If it does, then filter the moves
		if (originHasMove) {
			std::erase_if(legalMoves, [&](const Move& m) { return m.from != takeOriginIndex; });
		}
	}
	// If the transposition table contains this position, find the stored best move and put it at the front
	if (entry.hash == hash) {
		auto iterator = std::find_if(legalMoves.begin(), legalMoves.end(), [&](const Move& mv) {
			return mv.from == entry.bestMove.from && mv.to == entry.bestMove.to;
			});
		if (iterator != legalMoves.end()) std::iter_swap(legalMoves.begin(), iterator);
	}
	// Check that the vector can fit the offset position of the killer move when there is a TT match
	bool moveCanFit = (entry.hash == hash) < legalMoves.size();
	// If the killer move store has killer moves for this depth, and legalMoves has said killer moves stored
	// Put them in the first and second position (second and third if there was a TT match)
	if (killerMoves[depth][0].from != -1 && moveCanFit) {
		auto iterator = std::find_if(legalMoves.begin(), legalMoves.end(), [&](const Move& mv) {
			return mv.from == killerMoves[depth][0].from && mv.to == killerMoves[depth][0].to;
			});
		if (iterator != legalMoves.end()) std::iter_swap(
			entry.hash == hash ? legalMoves.begin() + 1 : legalMoves.begin(),
			iterator);
	}
	// Redo the check for the other move (1 position further)
	moveCanFit = ((entry.hash == hash) + 1) < legalMoves.size();
	if (killerMoves[depth][1].from != -1 && moveCanFit) {
		auto iterator = std::find_if(legalMoves.begin(), legalMoves.end(), [&](const Move& mv) {
			return mv.from == killerMoves[depth][1].from && mv.to == killerMoves[depth][1].to;
			});
		if (iterator != legalMoves.end()) std::iter_swap(
			entry.hash == hash ? legalMoves.begin() + 2 : legalMoves.begin() + 1,
			iterator);
	}
	// If there are no legal moves, the maximizing player lost
	if (legalMoves.empty()) return isMaximizing ? INT_MIN : INT_MAX;

	score = isMaximizing ? INT_MIN : INT_MAX;
	int moveNumber = 1;
	bool takesOnly = legalMoves[0].isCapture;
	bool isFirstMove = true;
	Move bestMove{};
	// If the move is not part of a take chain, perform standard minimax
	// Otherwise, if takes are available, only consider moves made by the piece that started the chain
	for (Move& move : legalMoves) {
		if (takeOriginIndex != -1 && takesOnly && takeOriginIndex != move.from) continue;
		// Make a copy of the current board and apply the move to it
		BitboardSet nextBoard = board;
		ApplyMoveOnBitboardSet(&nextBoard, &move);
		// Does the same piece have captures available to continue the chain
		bool continuesChain = false;
		if (move.isCapture && !move.isCrown) {
			unsigned int jumpsAfter = colour ? nextBoard.GetJumpersWhite() : nextBoard.GetJumpersBlack();
			continuesChain = (jumpsAfter >> move.to) & 1;
		}
		// Run minimax for the next move with the new position
		int eval;
		if (continuesChain) {
			// Same colour and depth because the chain is continued
			eval = Minimax(nextBoard, depth, colour, maximizingIsWhite, move.to, alpha, beta, false);
		}
		else {
			// The turn passes to the other player
			int searchDepth = depth - 1;
			// Decide whether the move qualifies for Late Move Reductions
			bool isReduced = moveNumber > LMR_MIN_COUNT && !move.isCrown && !move.isCapture && depth > LMR_MIN_DEPTH;
			// If the move qualifies, run with reduced depth first
			if (isReduced) {
				int nullAlpha = isMaximizing ? alpha : (beta - 1);
				int nullBeta = isMaximizing ? (alpha + 1) : beta;
				int reducedEval = Minimax(nextBoard, searchDepth - 1, !colour, maximizingIsWhite,
					takesOnly ? move.to : -1, nullAlpha, nullBeta, false);
				bool hasEscaped = isMaximizing ? (reducedEval > alpha) : (reducedEval < beta);
				// If the move has escaped the window, do regular PVS
				if (!hasEscaped) eval = reducedEval;
				else {
					// Run the first search with a full window, any subsequent searches are done with a null window first
					if (isFirstMove) {
						eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
						isFirstMove = false;
					}
					else {
						if ((beta - alpha) > 1) {
							// The window is not narrow enough, run the search with a null window
							int nullAlpha = isMaximizing ? alpha : (beta - 1);
							int nullBeta = isMaximizing ? (alpha + 1) : beta;
							eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, nullAlpha, nullBeta, false);
							// If the move was better than the first one, run a full search
							bool isBetter = isMaximizing ? (eval > alpha) : (eval < beta);
							if (isBetter) {
								eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
							}
						}
						else {
							// The window is already narrow, run a normal search
							eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
						}
					}
				}
			}
			// If the move doesn't qualify for LMR, run PVS normally
			else {
				// Run the first search with a full window, any subsequent searches are done with a null window first
				if (isFirstMove) {
					eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
					isFirstMove = false;
				}
				else {
					if ((beta - alpha) > 1) {
						// The window is not narrow enough, run the search with a null window
						int nullAlpha = isMaximizing ? alpha : (beta - 1);
						int nullBeta = isMaximizing ? (alpha + 1) : beta;
						eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, nullAlpha, nullBeta, false);
						// If the move was better than the first one, run a full search
						bool isBetter = isMaximizing ? (eval > alpha) : (eval < beta);
						if (isBetter) {
							eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
						}
					}
					else {
						// The window is already narrow, run a normal search
						eval = Minimax(nextBoard, depth - 1, !colour, maximizingIsWhite, takesOnly ? move.to : -1, alpha, beta, false);
					}
				}
			}
			
		}
		moveNumber++;
		// Update apha/beta and the score
		if (isMaximizing) {
			if (eval > score) {
				score = eval;
				bestMove = move;
			}
			
			// Update alpha if the new maximizing score is higher
			alpha = std::max(alpha, score);
		}
		else {
			if (eval < score) {
				score = eval;
				bestMove = move;
			}
			// Update beta if the new minimizing score is lower
			beta = std::min(beta, score);
		}
		// Prune if alpha is higher than/equal to beta
		if (alpha >= beta) {
			break;
		}
	}
	// Determine the type of bound for the score
	int bound;
	if (score <= originalAlpha) bound = 1;
	else if (score >= originalBeta) bound = -1;
	else bound = 0;
	// If the depth is higher than or equal to the stored depth, store the hash of the position
	if (depth >= transpositionTable[index].depth) transpositionTable[index] = { hash, depth, score, bound, bestMove };
	// If the loop broke early due to alpha-beta pruning store the move as a "killer move"
	// Do not store captures
	if (bound == -1 && !bestMove.isCapture) {
		killerMoves[depth][1] = killerMoves[depth][0];
		killerMoves[depth][0] = bestMove;
	}

	return score;
}

// Generate a move using the Minimax rule to find the best possible move for the bot with the given depth
AppliedMove Bot::GenerateMove(std::vector<Piece>* board_state, int depth,
	bool botColour, bool isCaptureChain, int forcedOriginX, int forcedOriginY) {
	NodeCounter = 0;
	AppliedMove result = { -1, -1, -1, -1, -1, -1, 0, 0 };
	Move chosenMove{};
	// Update the bitboards with the current board state and reset the killer moves array
	bitboards.UpdateBitboards(board_state);
	ResetKillerMoves();
	// Generate all of the possible legal moves for the current player (the bot)
	unsigned int movers = botColour ? bitboards.GetMoversWhite() : bitboards.GetMoversBlack();
	unsigned int jumpers = botColour ? bitboards.GetJumpersWhite() : bitboards.GetJumpersBlack();
	std::vector<Move> legalMoves = GenerateLegalMoves(bitboards, botColour, movers, jumpers);
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

	// Helper lambda function for running the search loop
	auto RunSearchLoop = [&](int& currentDepth, int windowAlpha, int windowBeta, int& highScore, int& bestMoveIndex) {
		bool isFirstMove = true;
		bool choseMoveThisPass = false;
		
		for (int i = 0; i < legalMoves.size(); i++) {
			const Move& move = legalMoves[i];
			BitboardSet nextBoard = bitboards;
			Move moveCopy = move;
			ApplyMoveOnBitboardSet(&nextBoard, &moveCopy);

			// Does the same piece have captures available to continue the chain
			bool continuesChain = false;
			if (move.isCapture && !move.isCrown) {
				unsigned int jumpsAfter = botColour ? nextBoard.GetJumpersWhite() : nextBoard.GetJumpersBlack();
				continuesChain = (jumpsAfter >> move.to) & 1;
			}

			int score;
			if (continuesChain) {
				score = Minimax(nextBoard, currentDepth, botColour, botColour, moveCopy.to, windowAlpha, windowBeta, false);
			}
			else {
				// Run the first search with a full window, any subsequent searches are done with a null window first
				if (isFirstMove) {
					score = Minimax(nextBoard, currentDepth - 1, !botColour, botColour, -1, windowAlpha, windowBeta, false);
					isFirstMove = false;
				}
				else {
					// First, run the search with a null window
					score = Minimax(nextBoard, currentDepth - 1, !botColour, botColour, -1, highScore, highScore + 1, false);
					// If the null search shows that the move is better than our current best, run the search with a full window
					if (score > highScore) {
						score = Minimax(nextBoard, currentDepth - 1, !botColour, botColour, -1, highScore, INT_MAX, false);
					}
				}
			}

			if (!choseMoveThisPass || score > highScore) {
				bestMoveIndex = i;
				highScore = score;
				choseMoveThisPass = true;
			}
		}
	};

	// Iterative deepening, run minimax one depth at a time, saving the best move of each last depth
	// and putting it at the front for the next pass
	int previousScore = 0;

	for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
		int highScore = INT_MIN;
		int bestMoveIndex = 0;
		// Aspiration Windows, save the score of the previous pass and run the next pass with a small window around that score
		// If the search fails, the best move is much better/worse than the best move of the last pass
		// and should be re-searched with the full window.
		// If it doesn't fail, much less resources are spent looking for it
		int windowAlpha = (currentDepth == 1) ? INT_MIN : previousScore - ASPIRATION_MARGIN;
		int windowBeta = (currentDepth == 1) ? INT_MAX : previousScore + ASPIRATION_MARGIN;
		
		// Run the loop with the set aspiration window
		RunSearchLoop(currentDepth, windowAlpha, windowBeta, highScore, bestMoveIndex);

		// Check if the aspiration window failed
		if (currentDepth > 1 && (highScore <= windowAlpha || highScore >= windowBeta)) {
			// Re-run the same depth with the default wide window
			windowAlpha = INT_MIN;
			windowBeta = INT_MAX;
			highScore = INT_MIN;

			RunSearchLoop(currentDepth, windowAlpha, windowBeta, highScore, bestMoveIndex);
		}

		chosenMove = legalMoves[bestMoveIndex];
		previousScore = highScore;
		// Move this pass's best move to the front
		std::swap(legalMoves[0], legalMoves[bestMoveIndex]);
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
	std::cout << NodeCounter << std::endl;
	return result;
}