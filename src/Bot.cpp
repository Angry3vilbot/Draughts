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

			int finalDestination = bitboards.CoordinatesToBitIndex(destinationX, destinationY);
			if (finalDestination < 32 && finalDestination >= 0 && (notOccupied >> destination) & 1) {
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

// Generate a move using the Minimax rule to find the best possible move for the bot with the given depth
void Bot::GenerateMove(std::vector<Piece>* board_state, int depth, bool botColour) {
	AppliedMove result;
	// Update the bitboards with the current board state
	bitboards.UpdateBitboards(board_state);
	// Generate all of the possible legal moves for the current player (the bot)
	std::vector<Move> legalMoves = GenerateLegalMoves(bitboards, botColour);
	// If there are no legal moves, the game is over
	for (const Move& move : legalMoves) {
		std::cout << std::endl;
		std::cout << "Move from : to: " << move.from << " : " << move.to << std::endl;
		std::cout << "Move isCapture: " << move.isCapture << std::endl;
		if(move.isCapture) std::cout << "Move capturedSquare: " << move.capturedSquare << std::endl;
		std::cout << "Move isCrown: " << move.isCrown << std::endl;
	}
	// NOTE: For minimaxing chains of takes, have minimax take two piece bit indexes as an argument.
	// Inside minimax: if the origin bit index != 1 => only look at the move that takes the enemy piece at the second bit index
	// Inside the caller: for every move, if the move was a take, generate legal moves.
	// For every take run minimax with the same colour, first bit index set as the piece's location, second set as the enemy piece's location
	// And then always run minimax with the different colour (normal route/not chaining [further])
}