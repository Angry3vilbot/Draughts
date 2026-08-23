#pragma once
class Piece
{
	public:
		Piece(int x, int y, bool isWhite);
		int getX();
		int getY();
		bool getIsWhite();
		bool getIsKing();
	
	private:
		int x_coord;
		int y_coord;
		bool isWhite;
		bool isKing;
};

