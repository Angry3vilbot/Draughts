#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <iostream>
#include <vector>
#include "Piece.h"
#include "BoardLayout.h"
#include "ValidMove.h"
#include "AppliedMove.h"
using namespace std;

namespace Colours {
	constexpr Color TILE_LIGHT = BEIGE,
		TILE_DARK = BROWN,
		BOARD = DARKBROWN,
		PIECE_WHITE = RAYWHITE,
		PIECE_BLACK = BLACK,
		PIECE_OUTLINE = ORANGE,
		MOVE_INDICATOR = LIGHTGRAY,
		TAKE_INDICATOR = RED;
}

struct MovementState {
	bool isDragging = false;
	bool isSelected = false;
	int index = -1;
};

// Draws the piece in the correct colour and crown status
void DrawPiece(Vector2 center, float size, bool isWhite, bool isKing) {
	// Draw the piece
	DrawCircleV(
		center,
		size,
		isWhite ? Colours::PIECE_WHITE : Colours::PIECE_BLACK);
	if (isKing) {
		// Draw the king indicator
		DrawCircleV(
			center,
			size * 0.4,
			isWhite ? Colours::PIECE_BLACK : Colours::PIECE_WHITE);
	}
}

// Draws the game board, an 8x8 grid of alternating dark and light squares, surrounded by a darker border 
void DrawBoard(vector<Piece> *board_state, BoardLayout &board_layout, MovementState mov) {
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
	// Draw the pieces, skips drawing the piece that is being dragged/moved by the player
	for (int i = 0; i < board_state->size(); i++) {
		if (i == mov.index) continue;
		Piece piece = (*board_state)[i];
		Vector2 center = board_layout.getSquareCenter(piece.getX(), piece.getY());
		
		DrawPiece(center, board_layout.pieceSize, piece.getIsWhite(), piece.getIsKing());
	}
	// Draw possible moves
	if (mov.isDragging || mov.isSelected) {
		Piece piece = (*board_state)[mov.index];
		vector<ValidMove> locations = computeValidMoves(*board_state, piece);
		bool forcedCapture = false;
		// Check if any move is a capture
		for (ValidMove move : locations) {
			if (move.isCapture) {
				forcedCapture = true;
				break;
			}
		}

		for (ValidMove move : locations) {
			Vector2 center = board_layout.getSquareCenter(move.x, move.y);
			if (forcedCapture) {
				if (move.isCapture) {
					DrawCircleV(
						center,
						board_layout.pieceSize * 0.4,
						Colours::TAKE_INDICATOR);
				}
			}
			else {
				DrawCircleV(
					center,
					board_layout.pieceSize * 0.4,
					Colours::MOVE_INDICATOR);
			}
		}
	}
	// Draw the piece being dragged
	if (mov.isDragging) {
		Piece piece = (*board_state)[mov.index];
		Vector2 mousePos = GetMousePosition();

		// Draw an outline
		DrawRing(
			mousePos,
			board_layout.pieceSize,
			board_layout.pieceSize * 1.1,
			0,
			360,
			0,
			Colours::PIECE_OUTLINE);
		DrawPiece(mousePos, board_layout.pieceSize, piece.getIsWhite(), piece.getIsKing());
	}
	// Draw the selected piece
	else if (mov.isSelected) {
		Piece piece = (*board_state)[mov.index];
		Vector2 center = board_layout.getSquareCenter(piece.getX(), piece.getY());
		
		// Draw an outline
		DrawRing(
			center,
			board_layout.pieceSize,
			board_layout.pieceSize * 1.1,
			0,
			360,
			0,
			Colours::PIECE_OUTLINE);
		DrawPiece(center, board_layout.pieceSize, piece.getIsWhite(), piece.getIsKing());
	}
}
// Handle moving the player's selected piece
void handleMovement(vector<Piece>* board_state, BoardLayout& board_layout, MovementState& mov, Vector2 mousePos) {
	Piece piece = (*board_state)[mov.index];
	vector<ValidMove> locations = computeValidMoves(*board_state, piece);

	for (ValidMove location : locations) {
		Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());
		Vector2 squareCorner = board_layout.getSquareTopLeft(location.x, location.y);
		Rectangle square = { squareCorner.x, squareCorner.y, board_layout.tileSize, board_layout.tileSize };

		bool collisionMouseSquare = CheckCollisionPointRec(mousePos, square);
		bool collisionPieceSquare = CheckCollisionCircleRec(pieceCenter, board_layout.pieceSize, square);

		if (collisionMouseSquare || collisionPieceSquare) {
			AppliedMove appliedMove = tryApplyMove(piece, location);

			if (appliedMove.isCapture) {
				int index;
				for (index = 0; index < board_state->size(); index++) {
					if ((*board_state)[index].getX() == appliedMove.captureX && (*board_state)[index].getY() == appliedMove.captureY) {
						board_state->erase(board_state->begin() + index);
						break;
					}
				}
			}

			for (Piece& current : *board_state) {
				if (current.getX() == appliedMove.sourceX && current.getY() == appliedMove.sourceY) {
					current.setX(appliedMove.destinationX);
					current.setY(appliedMove.destinationY);
					if (appliedMove.isCrown) current.setIsKing(true);
					break;
				}
			}

			break;
		}
	}
}
// Reads the mouse inputs of the player to move the pieces
void ReadInput(vector<Piece>* board_state, BoardLayout &board_layout, bool playerColour, MovementState &mov) {
	Vector2 mousePos = GetMousePosition();
	// Dragging pieces around
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (mov.isDragging) return;
		for (int i = 0; i < board_state->size(); i++) {
			Piece& piece = (*board_state)[i];
			Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());

			if (CheckCollisionPointCircle(mousePos, pieceCenter, board_layout.pieceSize) && piece.getIsWhite() == playerColour) {
				mov.isDragging = true;
				mov.index = i;
				break;
			}
		}
	}
	// Selecting a piece to move, try moving a selected piece if there is one
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if(mov.index >= 0) handleMovement(board_state, board_layout, mov, mousePos);

		bool hadCollision = false;
		for (int i = 0; i < board_state->size(); i++) {
			Piece& piece = (*board_state)[i];
			Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());

			if (CheckCollisionPointCircle(mousePos, pieceCenter, board_layout.pieceSize) && piece.getIsWhite() == playerColour) {
				mov.index = i;
				mov.isSelected = true;
				hadCollision = true;
				break;
			}
		}
		// If we had no collision, deselect the piece
		if (!hadCollision) {
			mov.index = -1;
			mov.isSelected = false;
		}
	}
	// Stops dragging the pieces, try moving the piece
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		mov.isDragging = false;
		mov.index = mov.isSelected ? mov.index : -1;

		if (mov.index >= 0) handleMovement(board_state, board_layout, mov, mousePos);
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
	MovementState mov;

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// Compute the board layout
		BoardLayout board_layout = computeLayout();
		// drawing
		BeginDrawing();
		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);
		DrawBoard(&board_state, board_layout, mov);
		ReadInput(&board_state, board_layout, playerColour, mov);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	return 0;
}