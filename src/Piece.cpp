#include "Piece.h"

Piece::Piece(int x, int y, bool isWhite)
: x_coord(x), y_coord(y), isWhite(isWhite), isKing(false)
{}

int Piece::getX()
{
	return x_coord;
}

int Piece::getY()
{
	return y_coord;
}

bool Piece::getIsWhite()
{
	return isWhite;
}

bool Piece::getIsKing()
{
	return isKing;
}
