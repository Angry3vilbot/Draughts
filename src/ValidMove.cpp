#include "ValidMove.h"

Piece* getPieceAt(int x, int y, std::vector<Piece>& board_state) {
    for (Piece& piece : board_state) {
        if (piece.getX() == x && piece.getY() == y) return &piece;
    }
    return nullptr;
}

std::vector<ValidMove> computeValidMoves(std::vector<Piece>& board_state, Piece& piece)
{
    std::vector<ValidMove> moves;
    int baseDirection = piece.getIsWhite() ? 1 : -1;
    std::vector<int> direction = piece.getIsKing() ? std::vector<int>{ 1, -1 } : std::vector<int>{ baseDirection };
    
    // Check one/both vertical directions
    for (int y : direction) {
        // Check both sideways directions
        for (int x : { 1, -1 }) {
            int targetY = piece.getY() + y;
            int targetX = piece.getX() + x;
            // Skip coordinates that are off the board
            if (targetX > 8 || targetY > 8 || targetX < 1 || targetY < 1) continue;

            Piece* target = getPieceAt(targetX, targetY, board_state);
            if (target != nullptr) {
                if (target->getIsWhite() == piece.getIsWhite()) continue;
                // Check if it's possible to take the piece (the space past this one is valid and free)
                targetY += y;
                targetX += x;
                if (targetX > 8 || targetY > 8 || targetX < 1 || targetY < 1) continue;

                if (getPieceAt(targetX, targetY, board_state) == nullptr) {
                    // Free space, is a valid capture
                    moves.push_back({ targetX, targetY, true });
                }
            }
            else {
                // Free space, is a valid move
                moves.push_back({ targetX, targetY, false });
            }
        }
    }

    return moves;
}
