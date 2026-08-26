#include "AppliedMove.h"

// Applies the player's move
AppliedMove tryApplyMove(Piece& piece, ValidMove& move) {
	AppliedMove result = { piece.getX(), piece.getY(), move.x, move.y, -1, -1, move.isCapture, false };
	if (move.isCapture) {
		result.captureX = abs(result.sourceX + piece.getIsWhite());
		result.captureY = abs(result.sourceY + piece.getIsWhite());
	}

	bool isWhiteAndCrowned = piece.getIsWhite() && result.destinationY == 8 && !piece.getIsKing();
	bool isBlackAndCrowned = !piece.getIsWhite() && result.destinationY == 1 && !piece.getIsKing();
	if (isWhiteAndCrowned || isBlackAndCrowned) {
		result.isCrown = true;
	}

	return result;
}
