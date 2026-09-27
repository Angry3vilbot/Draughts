#pragma once
#include "raylib.h"
#include "Piece.h"
#include <vector>
#include "BoardLayout.h"
#include "GameSettings.h"
#include "GameState.h"
#include "MoveResult.h"

namespace Colours {
	constexpr Color TILE_LIGHT = BEIGE,
		TILE_DARK = BROWN,
		BOARD = DARKBROWN,
		PIECE_WHITE = RAYWHITE,
		PIECE_BLACK = BLACK,
		PIECE_OUTLINE = ORANGE,
		MOVED_PIECE_TILE = { 255, 249, 0, 100 },
		MOVE_INDICATOR = LIGHTGRAY,
		TAKE_INDICATOR = RED;
}
struct MovementState {
	bool isDragging = false;
	bool isSelected = false;
	int index = -1;
};

extern MovementState mov;

void ResetMovementState();
void DrawPiece(Vector2 center, float size, bool isWhite, bool isKing);
void DrawBoard(std::vector<Piece>* board_state, BoardLayout& board_layout, GameState& game);
MoveResult handleMovement(std::vector<Piece>* board_state, BoardLayout& board_layout, Vector2 mousePos, AppliedMove& lastMove);
MoveResult ReadInput(std::vector<Piece>* board_state, BoardLayout& board_layout, GameState& game, std::vector<uint64_t>& history);
