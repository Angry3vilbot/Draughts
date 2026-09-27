#include "Result.h"

void DrawResultScreen(bool whiteDidWin, Bot& bot, bool& started) {
	const char* resultStr = whiteDidWin ? "White Won" : "Black Won";
	// Draw the overlay
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 100 });
	// Draw the text
	DrawText("GAME OVER", (GetScreenWidth() - MeasureText("GAME OVER", 64)) / 2, GetScreenHeight() * 0.2, 64, BLACK);
	DrawText(resultStr, (GetScreenWidth() - MeasureText(resultStr, 48)) / 2, GetScreenHeight() * 0.4, 48, BLACK);
	// Draw Play Again button
	DrawNewGameButton(bot);
	// Draw Main Menu button
	DrawMainMenuButton(bot, started);
}

void DrawNewGameButton(Bot& bot) {
	int btnWidth = 0.1 * GetScreenWidth(), btnHeight = 0.1 * GetScreenHeight();
	int btnX = (GetScreenWidth() - btnWidth) / 2 + btnWidth;
	int btnY = btnHeight * 6;
	float textSize = (GetScreenWidth() / 1920.0) * 32.0;
	int textX = btnX + (btnWidth / 2) - (MeasureText("Play Again", textSize) / 2);
	int textY = btnY + (btnHeight / 2) - 18;

	Rectangle button = { btnX, btnY, btnWidth, btnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("Play Again", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
		ResetBoardState();
		SoftResetAll(bot);
	}
}

void DrawMainMenuButton(Bot& bot, bool& started) {
	int btnWidth = 0.1 * GetScreenWidth(), btnHeight = 0.1 * GetScreenHeight();
	int btnX = (GetScreenWidth() - btnWidth) / 2 - btnWidth;
	int btnY = btnHeight * 6;
	float textSize = (GetScreenWidth() / 1920.0) * 32.0;
	int textX = btnX + (btnWidth / 2) - (MeasureText("Main Menu", textSize) / 2);
	int textY = btnY + (btnHeight / 2) - 18;

	Rectangle button = { btnX, btnY, btnWidth, btnHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, RAYWHITE);
	DrawRectangleLinesEx(button, 1, hover ? GREEN : BLACK);
	DrawText("Main Menu", textX, textY, textSize, BLACK);
	// Input Handling
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
		ResetBoardState();
		ResetAll(bot);
		started = false;
	}
}