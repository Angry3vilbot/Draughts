#include "AppliedMove.h"

// Applies the player's move
AppliedMove tryApplyMove(Piece& piece, ValidMove& move) {
	AppliedMove result = { piece.getX(), piece.getY(), move.x, move.y, -1, -1, move.isCapture, false };
	if (move.isCapture) {
		result.captureX = (result.sourceX + result.destinationX) / 2;
		result.captureY = (result.sourceY + result.destinationY) / 2;
	}

	bool isWhiteAndCrowned = piece.getIsWhite() && result.destinationY == 8 && !piece.getIsKing();
	bool isBlackAndCrowned = !piece.getIsWhite() && result.destinationY == 1 && !piece.getIsKing();
	if (isWhiteAndCrowned || isBlackAndCrowned) {
		result.isCrown = true;
	}

	return result;
}
