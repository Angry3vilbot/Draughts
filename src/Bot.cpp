#include "Bot.h"

Bot::Bot(std::vector<Piece>* board_state) : bitboards(board_state) {}

AppliedMove Bot::GenerateMove(std::vector<Piece>* board_state, int depth)
{
	return AppliedMove();
}


