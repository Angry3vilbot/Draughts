#pragma once
#include "raylib.h"
#include "BoardState.h"
#include "Game.h"

void DrawResultScreen(bool whiteDidWin, Bot& bot, bool& started, bool& appliedConfig);
void DrawNewGameButton(Bot& bot);
void DrawMainMenuButton(Bot& bot, bool& started, bool& appliedConfig);