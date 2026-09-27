#pragma once
#include "raylib.h"
#include "GameSettings.h"
// 0 = game mode selection, 1 = VS. CPU...
extern int screenID;
extern bool selectedWhiteColour;
extern GameSettings settings;

void DrawMenu(bool& started, GameSettings& settings);