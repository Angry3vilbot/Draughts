#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <iostream>
#include <vector>
#include "Piece.h"
#include "BoardLayout.h"
using namespace std;

namespace Colours {
	constexpr Color TILE_LIGHT = BEIGE,
		TILE_DARK = BROWN,
		BOARD = DARKBROWN,
		PIECE_WHITE = RAYWHITE,
		PIECE_BLACK = BLACK;
}

struct DraggingState {
	bool isDragging = false;
	int index = -1;
};

// Draws the game board, an 8x8 grid of alternating dark and light squares, surrounded by a darker border 
void DrawBoard(vector<Piece> *board_state, BoardLayout &board_layout, DraggingState drag) {
	// Draw background to serve as a border
	DrawRectangle(board_layout.boardX, board_layout.boardY, board_layout.boardSize, board_layout.boardSize, Colours::BOARD);
	// Draw the grid of squares
	for (int x = 1; x <= 8; x++) {
		for (int y = 1; y <= 8; y++) {
			Vector2 topLeft = board_layout.getSquareTopLeft(x, y);
			
			DrawRectangle(topLeft.x, topLeft.y, board_layout.tileSize, board_layout.tileSize,
				(x + y) % 2 == 0 ? Colours::TILE_DARK : Colours::TILE_LIGHT);
		}
	}
	// Draw the pieces, skips drawing the piece that is being dragged by the player
	for (int i = 0; i < board_state->size(); i++) {
		if (i == drag.index) continue;
		Piece piece = (*board_state)[i];

		Vector2 center = board_layout.getSquareCenter(piece.getX(), piece.getY());
		DrawCircle(
			(int) center.x,
			(int) center.y,
			board_layout.pieceSize,
			piece.getIsWhite() ? Colours::PIECE_WHITE : Colours::PIECE_BLACK);
	}
	// Draw the piece being dragged
	if (drag.isDragging) {
		Piece piece = (*board_state)[drag.index];
		Vector2 mousePos = GetMousePosition();

		DrawCircle(
			(int)mousePos.x,
			(int)mousePos.y,
			board_layout.pieceSize,
			piece.getIsWhite() ? Colours::PIECE_WHITE : Colours::PIECE_BLACK);
	}

}
// Reads the mouse inputs of the player to move the pieces
void ReadInput(vector<Piece>* board_state, BoardLayout &board_layout, bool playerColour, DraggingState &drag) {
	Vector2 mousePos = GetMousePosition();
	// Dragging pieces around
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (drag.isDragging) return;
		for (int i = 0; i < board_state->size(); i++) {
			Piece& piece = (*board_state)[i];
			Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());

			if (CheckCollisionPointCircle(mousePos, pieceCenter, board_layout.pieceSize) && piece.getIsWhite() == playerColour) {
				drag.isDragging = true;
				drag.index = i;
				break;
			}
		}
	}
	// Stops dragging the pieces
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		drag.isDragging = false;
		drag.index = -1;
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

	bool playerColour = true; // Temporary
	DraggingState drag;

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// Compute the board layout
		BoardLayout board_layout = computeLayout();
		// drawing
		BeginDrawing();
		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);
		DrawBoard(&board_state, board_layout, drag);
		ReadInput(&board_state, board_layout, playerColour, drag);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	return 0;
}