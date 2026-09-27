#include "Menu.h"
int screenID = 0;
bool selectedWhiteColour = true;
GameSettings settings{};

// Draws the VS. CPU button
void DrawCPUButton() {
	int menuBtnWidth = 0.2 * GetScreenWidth(), menuBtnHeight = 0.1 * GetScreenHeight();
	int menuBtnX = (GetScreenWidth() - menuBtnWidth) / 2;
	int menuBtnY = menuBtnHeight * 2;
	float textSize = (GetScreenWidth() / 1920.0) * 48.0;
	int textX = menuBtnX + (menuBtnWidth / 2) - (MeasureText("VS. CPU", textSize) / 2);
	int textY = menuBtnY + (menuBtnHeight / 2) - 18;

	Rectangle button = { menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("VS. CPU", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) screenID = 1;
}
// Draws the Local Play button
void DrawLocalButton(bool& started, GameSettings& settings) {
	int menuBtnWidth = 0.2 * GetScreenWidth(), menuBtnHeight = 0.1 * GetScreenHeight();
	int menuBtnX = (GetScreenWidth() - menuBtnWidth) / 2;
	int menuBtnY = menuBtnHeight * 4;
	float textSize = (GetScreenWidth() / 1920.0) * 48.0;
	int textX = menuBtnX + (menuBtnWidth / 2) - (MeasureText("Local Play", textSize) / 2);
	int textY = menuBtnY + (menuBtnHeight / 2) - 18;

	Rectangle button = { menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("Local Play", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
		settings.choseWhite = selectedWhiteColour;
		settings.gameMode = 2;
		started = true;
	}
}
// Draws the colour selection
void DrawColourSelection() {
	int colourBtnSize = 0.1 * GetScreenWidth();
	int colourBtnBaseX = (GetScreenWidth() - colourBtnSize) / 2;
	int colourBtnY = GetScreenHeight() * 0.2;
	int pieceSize = ((GetScreenWidth() / 2) - (GetScreenWidth() / 2 * 0.05)) / 20;

	Rectangle whiteBtn = { colourBtnBaseX - colourBtnSize, colourBtnY, colourBtnSize, colourBtnSize };
	Rectangle blackBtn = { colourBtnBaseX + colourBtnSize, colourBtnY, colourBtnSize, colourBtnSize };
	Vector2 mousePos = GetMousePosition();
	bool hoverWhite = CheckCollisionPointRec(mousePos, whiteBtn);
	bool hoverBlack = CheckCollisionPointRec(mousePos, blackBtn);
	Vector2 whitePiece = { (colourBtnBaseX - colourBtnSize) + colourBtnSize / 2, colourBtnY + colourBtnSize / 2 };
	Vector2 blackPiece = { (colourBtnBaseX + colourBtnSize) + colourBtnSize / 2, colourBtnY + colourBtnSize / 2 };

	DrawText("Colour", (GetScreenWidth() - MeasureText("Colour", 24)) / 2, GetScreenHeight() * 0.15, 24, BLACK);

	DrawRectangleRec(whiteBtn, RAYWHITE);
	DrawRectangleLinesEx(whiteBtn, 2, (hoverWhite || selectedWhiteColour) ? GREEN : BLACK);
	DrawCircleV(whitePiece, pieceSize, RAYWHITE);
	DrawRing(whitePiece, pieceSize, pieceSize * 1.1, 0, 360, 0, BLACK);

	DrawRectangleRec(blackBtn, RAYWHITE);
	DrawRectangleLinesEx(blackBtn, 2, (hoverBlack || !selectedWhiteColour) ? GREEN : BLACK);
	DrawCircleV(blackPiece, pieceSize, BLACK);
	// Input Handling
	if (hoverWhite && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		selectedWhiteColour = true;
	}
	else if (hoverBlack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		selectedWhiteColour = false;
	}
}
// Draws the difficulty (depth) selector
void DrawDifficultySelection(GameSettings& settings) {
	int scaleW = GetScreenWidth() * 0.3;
	int scaleH = GetScreenHeight() * 0.05;
	int scaleX = (GetScreenWidth() - scaleW) / 2;
	int scaleY = GetScreenHeight() * 0.5;
	Rectangle scale = { scaleX, scaleY, scaleW, scaleH };

	float textSize = (GetScreenWidth() / 1920.0) * 24.0;
	const char* tooltipText = TextFormat("CPU Level: %d", settings.depth);
	int tooltipX = (GetScreenWidth() - MeasureText(tooltipText, textSize)) / 2;
	int tooltipY = GetScreenHeight() * 0.5 - scaleH;

	DrawText(tooltipText, tooltipX, tooltipY, textSize, BLACK);
	DrawRectangleRec(scale, GRAY);
	DrawRectangle(scaleX, scaleY, scaleW / 20 * settings.depth, scaleH, RED);
	DrawRectangleLinesEx(scale, 5, BLACK);
	DrawRectangle(
		scaleX + (scaleW / 20 * settings.depth),
		scaleY + (scaleH - GetScreenHeight() * 0.06) / 2,
		GetScreenWidth() * 0.01,
		GetScreenHeight() * 0.06,
		LIGHTGRAY);

	// Input Handling
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		Vector2 mousePos = GetMousePosition();
		if (CheckCollisionPointRec(mousePos, scale)) {
			float selection = (mousePos.x - scale.x) / scale.width;
			settings.depth = (int)(selection * 20 + 0.5);

			if (settings.depth < 1) settings.depth = 1;
			if (settings.depth > 20) settings.depth = 20;
		}
	}
}
// Draws the button that starts the game
void DrawPlayButton(bool& started, GameSettings& settings) {
	int menuBtnWidth = 0.2 * GetScreenWidth(), menuBtnHeight = 0.1 * GetScreenHeight();
	int menuBtnX = (GetScreenWidth() - menuBtnWidth) / 2;
	int menuBtnY = menuBtnHeight * 6;
	float textSize = (GetScreenWidth() / 1920.0) * 36.0;
	int textX = menuBtnX + (menuBtnWidth / 2) - (MeasureText("Play", textSize) / 2);
	int textY = menuBtnY + (menuBtnHeight / 2) - 18;

	Rectangle button = { menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("Play", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
		settings.choseWhite = selectedWhiteColour;
		settings.gameMode = 1;
		started = true;
	}
}
// Draw the back button
void DrawBackButton() {
	int menuBtnWidth = 0.2 * GetScreenWidth(), menuBtnHeight = 0.1 * GetScreenHeight();
	int menuBtnX = (GetScreenWidth() - menuBtnWidth) / 2;
	int menuBtnY = menuBtnHeight * 8;
	float textSize = (GetScreenWidth() / 1920.0) * 36.0;
	int textX = menuBtnX + (menuBtnWidth / 2) - (MeasureText("Back", textSize) / 2);
	int textY = menuBtnY + (menuBtnHeight / 2) - 18;

	Rectangle button = { menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("Back", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
		screenID = 0;
	}
}
// Draws the main menu of the game
void DrawMenu(bool& started, GameSettings& settings) {
	DrawText("DRAUGHTS", (GetScreenWidth() - MeasureText("DRAUGHTS", 64)) / 2, GetScreenHeight() * 0.05, 64, BLACK);
	switch (screenID) {
	case 0:
		DrawCPUButton();
		DrawLocalButton(started, settings);
		break;
	case 1:
		DrawColourSelection();
		DrawDifficultySelection(settings);
		DrawPlayButton(started, settings);
		DrawBackButton();
		break;
	}
	DrawText("Press ESC to exit", (GetScreenWidth() - MeasureText("Press ESC to exit", 20)) / 2, GetScreenHeight() * 0.95, 20, BLACK);
}