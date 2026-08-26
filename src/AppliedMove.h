#pragma once
#include "Piece.h"
#include "ValidMove.h"

struct AppliedMove {
	int sourceX, sourceY, destinationX, destinationY, captureX, captureY;
	bool isCapture, isCrown;
};

AppliedMove tryApplyMove(Piece& piece, ValidMove& move);