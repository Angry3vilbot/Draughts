#pragma once
#include <cstdint>
#include "BitboardSet.h"
// Indices used for hashing a specific piece
constexpr int whitePiece = 0, whiteKing = 1, blackPiece = 2, blackKing = 3;
// Hash for every type of piece at every possible square
extern uint64_t pieceTable[32][4];
// Hash for every square for the capture chain origin
extern uint64_t chainOriginTable[32];
// Hash for which side's turn it is. XOR'ed in if White is to move
extern uint64_t sideToMoveHash;

void InitZobristHash();
uint64_t ComputeZobristHash(const BitboardSet& board, bool colour, int takeOriginIndex);