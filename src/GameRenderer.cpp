#include "GameRenderer.h"

MovementState mov{};

void ResetMovementState() {
	mov.isDragging = false;
	mov.isSelected = false;
	mov.index = -1;
}
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
void DrawBoard(std::vector<Piece>* board_state, BoardLayout& board_layout, GameState& game) {
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
	// Draw the indicator for the move that was just made, if there was one
	if (game.lastMove.sourceX != -1) {
		Vector2 topLeftOrigin = board_layout.getSquareTopLeft(game.lastMove.sourceX, game.lastMove.sourceY);
		Vector2 topLeftDest = board_layout.getSquareTopLeft(game.lastMove.destinationX, game.lastMove.destinationY);

		// Tint the origin and the destination square
		DrawRectangle(topLeftOrigin.x, topLeftOrigin.y, board_layout.tileSize, board_layout.tileSize, Colours::MOVED_PIECE_TILE);
		DrawRectangle(topLeftDest.x, topLeftDest.y, board_layout.tileSize, board_layout.tileSize, Colours::MOVED_PIECE_TILE);
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
		std::vector<ValidMove> locations = computeValidMoves(*board_state, piece);
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
MoveResult handleMovement(std::vector<Piece>* board_state, BoardLayout& board_layout, Vector2 mousePos, AppliedMove& lastMove) {
	MoveResult result;
	Piece piece = (*board_state)[mov.index];
	std::vector<ValidMove> locations = computeValidMoves(*board_state, piece);

	for (ValidMove location : locations) {
		Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());
		Vector2 squareCorner = board_layout.getSquareTopLeft(location.x, location.y);
		Rectangle square = { squareCorner.x, squareCorner.y, board_layout.tileSize, board_layout.tileSize };

		bool collisionMouseSquare = CheckCollisionPointRec(mousePos, square);
		bool collisionPieceSquare = CheckCollisionCircleRec(pieceCenter, board_layout.pieceSize, square);

		if (collisionMouseSquare || collisionPieceSquare) {
			AppliedMove appliedMove = tryApplyMove(piece, location);
			lastMove = appliedMove;

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

			result.moved = true;
			result.isCapture = appliedMove.isCapture;
			result.isCrown = appliedMove.isCrown;
			result.destinationX = appliedMove.destinationX;
			result.destinationY = appliedMove.destinationY;
			break;
		}
	}
	return result;
}
// Reads the mouse inputs of the player to move the pieces
MoveResult ReadInput(std::vector<Piece>* board_state, BoardLayout& board_layout, GameState& game, std::vector<uint64_t>& history) {
	Vector2 mousePos = GetMousePosition();
	MoveResult moveResult{};
	// Dragging pieces around
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (mov.isDragging) return moveResult;

		bool areCapturesAvailable = AnyPieceHasCaptures(*board_state, game.playerColour);
		for (int i = 0; i < board_state->size(); i++) {
			Piece& piece = (*board_state)[i];
			Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());
			// If it's a capture chain, check if the piece is the one that initiated it
			// Otherwise, if there are any captures available, only match pieces with captures
			bool isMatch;
			if (game.isCaptureChain) {
				isMatch = piece.getX() == game.chainOriginX && piece.getY() == game.chainOriginY;
			}
			else {
				isMatch = !areCapturesAvailable || PieceHasCaptures(*board_state, piece);
			}

			if (CheckCollisionPointCircle(mousePos, pieceCenter, board_layout.pieceSize) && piece.getIsWhite() == game.playerColour
				&& isMatch) {
				mov.isDragging = true;
				mov.index = i;
				break;
			}
		}
	}
	// Selecting a piece to move, try moving a selected piece if there is one
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (mov.index >= 0) {
			moveResult = handleMovement(board_state, board_layout, mousePos, game.lastMove);
			if (moveResult.moved) {
				ResetMovementState();
				return moveResult;
			}
		}
		// Select a piece
		bool hadCollision = false;
		bool areCapturesAvailable = AnyPieceHasCaptures(*board_state, game.playerColour);
		for (int i = 0; i < board_state->size(); i++) {
			Piece& piece = (*board_state)[i];
			Vector2 pieceCenter = board_layout.getSquareCenter(piece.getX(), piece.getY());
			// If it's a capture chain, check if the piece is the one that initiated it
			// Otherwise, if there are any captures available, only match pieces with captures
			bool isMatch;
			if (game.isCaptureChain) isMatch = piece.getX() == game.chainOriginX && piece.getY() == game.chainOriginY;
			else isMatch = !areCapturesAvailable || PieceHasCaptures(*board_state, piece);

			if (CheckCollisionPointCircle(mousePos, pieceCenter, board_layout.pieceSize) && piece.getIsWhite() == game.playerColour && isMatch) {
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

		if (mov.index >= 0) {
			moveResult = handleMovement(board_state, board_layout, mousePos, game.lastMove);
			if (moveResult.moved) {
				ResetMovementState();
				return moveResult;
			}
		}
	}
	return moveResult;
}
