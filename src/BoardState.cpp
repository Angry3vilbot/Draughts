#include "BoardState.h"

std::vector<Piece> board_state;

void InitBoardState() {
	board_state.reserve(24);
	ResetBoardState();
}

void ResetBoardState() {
	board_state.clear();

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
}