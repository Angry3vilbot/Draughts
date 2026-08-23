#pragma once
#include "raylib.h"

struct BoardLayout {
	int boardSize;
	// Border size used to calculate the gap between the edge of the board and the start of the tile grid
	int borderSize;
	int boardX;
	int boardY;
	int tileSize;
	int pieceSize;
	
	Vector2 getSquareTopLeft(int x, int y) {
		return {
			(float) ((boardX + borderSize / 2) + tileSize * (x - 1)),
			(float) ((boardY + borderSize / 2) + tileSize * (8 - y))
		};
	}

	Vector2 getSquareCenter(int x, int y) {
		Vector2 topLeft = getSquareTopLeft(x, y);
		return {
			topLeft.x + tileSize / 2,
			topLeft.y + tileSize / 2
		};
	}
};

inline BoardLayout computeLayout() {
	BoardLayout layout;

	layout.boardSize = GetScreenWidth() / 2;
	layout.borderSize = layout.boardSize * 0.05;
	layout.boardX = (GetScreenWidth() - layout.boardSize) / 2;
	layout.boardY = (GetScreenHeight() - layout.boardSize) / 2;
	layout.tileSize = (layout.boardSize - layout.borderSize) / 8;
	layout.pieceSize = layout.tileSize / 2.5;

	return layout;
}