#pragma once
#include "AppliedMove.h"
#include "MoveResult.h"
#include "GameSettings.h"
#include "BitboardSet.h"
#include "Zobrist.h"
#include "Bot.h"
#include <vector>
#include "GameState.h"
#include "BoardState.h"

extern std::vector<uint64_t> history;
extern GameState game;

void ResetAll(Bot& bot);
void SoftResetAll(Bot& bot);
void ResolveMoveOutcome(const MoveResult& moveResult, std::vector<Piece>* board_state);
void DoBotMove(std::vector<Piece>* board_state, Bot& bot);
void CheckLoss(std::vector<Piece>* board_state, bool colour);