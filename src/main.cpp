#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <iostream>
#include <vector>
#include "Piece.h"
using namespace std;

namespace Colours{
	constexpr Color TILE_LIGHT = BEIGE,
		TILE_DARK = BROWN,
		BOARD = DARKBROWN,
		PIECE_WHITE = RAYWHITE,
		PIECE_BLACK = BLACK;
}

// Draws the game board, an 8x8 grid of alternating dark and light squares, surrounded by a darker border 
void DrawBoard(vector<Piece> *board_state) {
	int boardSize = GetScreenWidth() / 2;
	// Border size used to calculate the gap between the edge of the board and the start of the tile grid
	int borderSize = boardSize * 0.05;
	int tileSize = (boardSize - borderSize) / 8;
	int pieceSize = tileSize / 2.5;

	int boardX = (GetScreenWidth() - boardSize) / 2;
	int boardY = (GetScreenHeight() - boardSize) / 2;
	// Draw background to serve as a border
	DrawRectangle(boardX, boardY, boardSize, boardSize, Colours::BOARD);
	// Draw the grid of squares
	for (int x = 1; x <= 8; x++) {
		for (int y = 1; y <= 8; y++) {
			int posX = (boardX + borderSize / 2) + tileSize * (x - 1);
			int posY = (boardY + borderSize / 2) + tileSize * (8 - y);
			
			DrawRectangle(posX, posY, tileSize, tileSize, (x + y) % 2 == 0 ? Colours::TILE_DARK : Colours::TILE_LIGHT);
		}
	}
	// Draw the pieces
	for (Piece piece : *board_state) {
		int squareX = (boardX + borderSize / 2) + tileSize * (piece.getX() - 1);
		int squareY = (boardY + borderSize / 2) + tileSize * (8 - piece.getY());
		DrawCircle(
			squareX + tileSize / 2,
			squareY + tileSize / 2,
			pieceSize,
			piece.getIsWhite() ? Colours::PIECE_WHITE : Colours::PIECE_BLACK);
	}
}

int main() {
	// Tell the resizable window to use vsync and work on high DPI displays, MSAA to remove aliasing from pieces
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

	// Create the window and OpenGL context
	InitWindow(1280, 720, "Draughts (Checkers)");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Initialize the board state
	vector<Piece> board_state;
	board_state.reserve(24);
	for (int y = 1; y <= 3; y++) {
		for (int x = 1; x <= 8; x++) {
			if ((x + y) % 2 == 0) {
				// Place white piece
				board_state.emplace_back(x, y, true);
			}
			if ((x + 9 - y) % 2 == 0) {
				// Place black piece
				board_state.emplace_back(x, 9 - y, false);
			}
		}
	}

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();
		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);
		DrawBoard(&board_state);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	return 0;
}