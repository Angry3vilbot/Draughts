#pragma once
#include "AppliedMove.h"
struct GameState {
	bool playerColour = false;
	bool playerTurn = playerColour;
	bool isCaptureChain = false;
	int chainOriginX = -1, chainOriginY = -1;
	AppliedMove lastMove{ -1, -1, -1, -1, -1, -1, false, false };
	int kingMoveCounter = 0;
	int eval = 0;
	int depth = 1;
	int status = 0; // -1 = black won, 1 = white won
	int gameMode = 1; // 1 = vs cpu, 2 = local...
};