#include "Piece.h"

Piece::Piece(int x, int y, bool isWhite)
: x_coord(x), y_coord(y), isWhite(isWhite), isKing(false)
{}

int Piece::getX()
{
	return x_coord;
}
void Piece::setX(int x)
{
	x_coord = x;
}

int Piece::getY()
{
	return y_coord;
}
void Piece::setY(int y)
{
	y_coord = y;
}

bool Piece::getIsWhite()
{
	return isWhite;
}

bool Piece::getIsKing()
{
	return isKing;
}
void Piece::setIsKing(bool isKing)
{
	this->isKing = isKing;
}
