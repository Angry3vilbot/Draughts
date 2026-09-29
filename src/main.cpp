#include "raylib.h"
#include <vector>
#include "BitboardMasks.h"
#include "Bot.h"
#include "Menu.h"
#include "Result.h"
#include "Game.h"
#include "GameRenderer.h"
#include "BoardState.h"
using namespace std;

int main() {
	// Tell the resizable window to use vsync and work on high DPI displays, MSAA to remove aliasing from pieces
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

	// Create the window and OpenGL context
	InitWindow(1280, 720, "Draughts (Checkers)");

	// Initialize the board state
	InitBoardState();

	InitBitboardMasks();
	InitZobristHash();
	history.reserve(256);
	Bot bot = Bot(&board_state);

	bool started = false;
	bool appliedConfig = false;
	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// Display the main menu until the user starts the game
		if (!started) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawMenu(started, settings);
			EndDrawing();
			continue;
		}
		else if (!appliedConfig) {
			game.playerColour = settings.choseWhite;
			game.playerTurn = game.playerColour;
			game.depth = settings.depth;
			game.gameMode = settings.gameMode;

			appliedConfig = true;
		}
		// Compute the board layout
		BoardLayout board_layout = computeLayout();
		// drawing
		BeginDrawing();
		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);
		if(game.gameMode == 1) DrawEval((float)game.eval / 100, board_layout, game.playerColour);
		DrawBoard(&board_state, board_layout, game);
		if (game.status != 0) {
			DrawResultScreen(game.status == 1, bot, started, appliedConfig);
			EndDrawing();
			continue;
		}
		// end the frame and get ready for the next one (display frame, poll input, etc...)
		EndDrawing();
		if (game.playerTurn) {
			MoveResult moveResult = ReadInput(&board_state, board_layout, game, history);
			if (moveResult.moved) ResolveMoveOutcome(moveResult, &board_state);
		}
		else {
			// Playing against the bot
			if (settings.gameMode == 1) {
				DoBotMove(&board_state, bot);
			}
			// Two Player Mode
			else {
				MoveResult moveResult = ReadInput(&board_state, board_layout, game, history);
				if (moveResult.moved) ResolveMoveOutcome(moveResult, &board_state);
			}
		}
	}

	return 0;
}