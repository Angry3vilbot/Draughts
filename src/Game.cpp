#include "Game.h"
std::vector<uint64_t> history;
GameState game;

// Resolve the outcome of the move: if it's a capture chain, check if it can be continued, otherwise end the turn
void ResolveMoveOutcome(const MoveResult& moveResult, std::vector<Piece>* board_state) {
	if (moveResult.isCapture && !moveResult.isCrown) {
		// Start/continue the capture chain
		game.isCaptureChain = true;
		game.chainOriginX = moveResult.destinationX;
		game.chainOriginY = moveResult.destinationY;
		// If the piece has more captures, continue the chain
		// Otherwise, end the player's turn
		bool hasFurtherCapture = false;
		for (Piece& piece : *board_state) {
			if (piece.getX() == moveResult.destinationX && piece.getY() == moveResult.destinationY) {
				for (ValidMove& move : computeValidMoves(*board_state, piece)) {
					if (move.isCapture) {
						hasFurtherCapture = true;
						break;
					}
				}
				break;
			}
		}

		if (!hasFurtherCapture) {
			game.isCaptureChain = false;
			game.playerTurn = !game.playerTurn;
			// If we're not in a capture chain, hash the position and save it into the history
			if (!game.isCaptureChain) {
				BitboardSet postMoveBoard = BitboardSet(board_state);
				uint64_t hash = ComputeZobristHash(postMoveBoard, !game.playerColour, -1);
				history.push_back(hash);
			}
		}
	}
	else {
		game.isCaptureChain = false;
		game.playerTurn = !game.playerTurn;
		// If we're not in a capture chain, hash the position and save it into the history
		if (!game.isCaptureChain) {
			BitboardSet postMoveBoard = BitboardSet(board_state);
			uint64_t hash = ComputeZobristHash(postMoveBoard, !game.playerColour, -1);
			history.push_back(hash);
		}
	}

	CheckLoss(board_state, !game.playerColour);
	if(game.status == 0) CheckLoss(board_state, game.playerColour);

	if (game.status == 0 && game.gameMode == 2 && !game.isCaptureChain) {
		game.playerColour = !game.playerColour;
	}
}
// Reset the game
void ResetAll(Bot& bot) {
	game = GameState{};
	history.clear();
	bot.ResetBot(&board_state);
}
// Reset the game, keeping the game settings
void SoftResetAll(Bot& bot) {
	int depth = game.depth;
	int gameMode = game.gameMode;
	bool playerColour =  game.playerColour;

	game = GameState{};
	history.clear();
	bot.ResetBot(&board_state);

	game.depth = depth;
	game.gameMode = gameMode;
	if (gameMode == 2) game.playerColour = true;
	else game.playerColour = !playerColour;
	game.playerTurn = game.playerColour;
}
// Generate and apply the bot's move
void DoBotMove(std::vector<Piece>* board_state, Bot& bot) {
	AppliedMove botMove = bot.GenerateMove(board_state, game.depth, !game.playerColour,
		game.isCaptureChain, game.chainOriginX, game.chainOriginY, history);
	if (botMove.sourceX == -1) {
		// The bot lost the game
		game.status = game.playerColour ? 1 : -1;
		return;
	}
	for (Piece& current : *board_state) {
		if (current.getX() == botMove.sourceX && current.getY() == botMove.sourceY) {
			current.setX(botMove.destinationX);
			current.setY(botMove.destinationY);
			if (botMove.isCrown) current.setIsKing(true);
			break;
		}
	}
	game.lastMove = botMove;
	if (botMove.isCapture) {
		std::erase_if(*board_state,
			[botMove](Piece piece) { return piece.getX() == botMove.captureX && piece.getY() == botMove.captureY; });
		// If the piece is crowned, the chain ends
		if (botMove.isCrown) {
			game.isCaptureChain = false;
			game.playerTurn = true;
		}
		else
		{
			for (Piece& piece : *board_state) {
				if (piece.getX() == botMove.destinationX && piece.getY() == botMove.destinationY) {
					// Check if there are captures available to continue the capture chain
					// If there aren't switch the turn to the player
					std::vector<ValidMove> nextMoves = computeValidMoves(*board_state, piece);
					game.isCaptureChain = false;
					for (ValidMove vm : nextMoves) {
						if (vm.isCapture) {
							game.isCaptureChain = true;
							break;
						}
					}
					if (game.isCaptureChain) {
						game.chainOriginX = botMove.destinationX;
						game.chainOriginY = botMove.destinationY;
					}
					game.playerTurn = !game.isCaptureChain;
					break;
				}
			}
		}
	}
	else {
		game.playerTurn = true;
	}
	// If we're not in a capture chain, hash the position and save it into the history
	if (!game.isCaptureChain) {
		BitboardSet postMoveBoard = BitboardSet(board_state);
		uint64_t hash = ComputeZobristHash(postMoveBoard, game.playerColour, -1);
		history.push_back(hash);
	}
	// Check if the player has legal moves available
	CheckLoss(board_state, game.playerColour);
	// If they do, check if the bot has legal moves available
	if(game.status == 0) CheckLoss(board_state, !game.playerColour);
}
// Checks if the given colour lost the game. If one has, sets the game status accordingly.
void CheckLoss(std::vector<Piece>* board_state, bool colour) {
	// Check if the colour has legal moves available
	bool hasValidMoves = false;
	for (Piece piece : *board_state) {
		if (piece.getIsWhite() == colour && computeValidMoves(*board_state, piece).size() > 0) {
			hasValidMoves = true;
			break;
		}
	}
	if (!hasValidMoves) game.status = colour ? -1 : 1;
}