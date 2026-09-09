#pragma once
#include <cstdint>
// Represents one entry in the Transposition Table, used to save evaluation results of positions for later reuse
struct TTEntry {
	// The Zobrist hash of the position of this entry in the Transposition Table.
	// The Transposition Table is represented by a hashmap. Zobrist hashing is used as the hash function.
	// However, only the lower bits are used to calculate the index, so you need to store the entire hash to make sure
	// that the table you're looking at has your actual position instead of a collision.
	uint64_t hash = 0;
	// The depth with which analysis from this position was made. The higher the value, the further the position was analyzed.
	// The further the position was analyzed, the more accurate the table entry. If the table entry is as deep or more
	// than the current position, this entry can be used instead of calculating a new analysis from scratch.
	int depth = -1;
	// The score that the position was evaluated as.
	int score = 0;
	// What does the score represent.
	// 0 means an exact score, the one computed after the search fully explored every leaf node without early cutoff.
	// 1 means it's the upper bound, the score was computed at a minimizing node, and it was cut off because the maximizing player would never choose a route that bad.
	// -1 means it's the lower bound, the score was computed at a maximizing node, and it was cut off because the minimizing player would never choose a route that bad.
	int bound = 0;
	// The best move of this position calculated at this depth
	Move bestMove{};
};