#pragma once
struct MoveResult {
	bool moved = false;
	bool isCapture = false;
	bool isCrown = false;
	int destinationX = -1, destinationY = -1;
};