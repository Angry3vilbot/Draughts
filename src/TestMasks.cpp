#include <vector>
#include "ValidMove.h"
#include "BitboardSet.h"
#include <iostream>

// Returns a bitboard of every piece of the given colour that has at least one
// legal quiet move, computed the slow-but-obviously-correct way via coordinates.
//unsigned int SlowGetMovers(std::vector<Piece>& board_state, bool isWhite, BitboardSet board) {
//	unsigned int result = 0;
//	for (Piece& piece : board_state) {
//		if (piece.getIsWhite() != isWhite) continue;
//		std::vector<ValidMove> moves = computeValidMoves(board_state, piece);
//		for (ValidMove& m : moves) {
//			if (!m.isCapture) {
//				result |= (1u << board.CoordinatesToBitIndex(piece.getX(), piece.getY()));
//				break;
//			}
//		}
//	}
//	return result;
//}

unsigned int SlowGetMovers(std::vector<Piece>& board_state, bool isWhite, BitboardSet& board) {
	unsigned int result = 0;
	for (Piece& piece : board_state) {
		if (piece.getIsWhite() != isWhite) continue;

		std::vector<int> dys;
		if (piece.getIsWhite() || piece.getIsKing()) dys.push_back(1);
		if (!piece.getIsWhite() || piece.getIsKing()) dys.push_back(-1);

		bool hasQuietMove = false;
		for (int dy : dys) {
			for (int dx : {-1, 1}) {
				int nx = piece.getX() + dx, ny = piece.getY() + dy;
				if (nx < 1 || nx > 8 || ny < 1 || ny > 8) continue;
				bool occupied = false;
				for (Piece& other : board_state) {
					if (other.getX() == nx && other.getY() == ny) { occupied = true; break; }
				}
				if (!occupied) { hasQuietMove = true; break; }
			}
			if (hasQuietMove) break;
		}

		if (hasQuietMove) {
			result |= (1u << board.CoordinatesToBitIndex(piece.getX(), piece.getY()));
		}
	}
	return result;
}

void main2() {
	InitBitboardMasks();
	srand(12345);
	int kingMismatches = 0, nonKingMismatches = 0;
	int trials = 200;

	for (int t = 0; t < trials; t++) {
		std::vector<Piece> board;
		for (int y = 1; y <= 8; y++) {
			for (int x = 1; x <= 8; x++) {
				if ((x + y) % 2 != 0) continue;
				int r = rand() % 100;
				if (r < 20) {
					Piece p(x, y, true);
					if (rand() % 100 < 30) p.setIsKing(true);
					board.push_back(p);
				}
				else if (r < 40) {
					Piece p(x, y, false);
					if (rand() % 100 < 30) p.setIsKing(true);
					board.push_back(p);
				}
			}
		}

		BitboardSet bitboards(&board);
		unsigned int fastWhite = bitboards.GetMoversWhite();
		unsigned int slowWhite = SlowGetMovers(board, true, bitboards);
		unsigned int fastBlack = bitboards.GetMoversBlack();
		unsigned int slowBlack = SlowGetMovers(board, false, bitboards);

		unsigned int diffWhite = fastWhite ^ slowWhite;
		unsigned int diffBlack = fastBlack ^ slowBlack;

		bool printedThisTrial = false;
		for (Piece& p : board) {
			int idx = bitboards.CoordinatesToBitIndex(p.getX(), p.getY());
			unsigned int diff = p.getIsWhite() ? diffWhite : diffBlack;
			if ((diff >> idx) & 1) {
				if (p.getIsKing()) kingMismatches++; else nonKingMismatches++;
				if (!printedThisTrial) {
					std::cout << "Trial " << t << " mismatch. Board:" << std::endl;
					for (Piece& pp : board) {
						std::cout << "  (" << pp.getX() << "," << pp.getY() << ") white=" << pp.getIsWhite()
							<< " king=" << pp.getIsKing() << std::endl;
					}
					printedThisTrial = true;
				}
				std::cout << "  Mismatched piece: (" << p.getX() << "," << p.getY() << ") white=" << p.getIsWhite()
					<< " king=" << p.getIsKing() << std::endl;
			}
		}
	}

	std::cout << "King mismatches: " << kingMismatches << ", non-king mismatches: " << nonKingMismatches << std::endl;
}