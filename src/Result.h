#pragma once
#include "raylib.h"
#include "BoardState.h"
#include "Game.h"

void DrawResultScreen(bool whiteDidWin, Bot& bot, bool& started);
void DrawNewGameButton(Bot& bot);
void DrawMainMenuButton(Bot& bot, bool& started);