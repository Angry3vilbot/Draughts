# Draughts
A game of English Draughts, as well as an engine, made in C++ with Raylib
## Features
- Simple, easy-to-use UI
- Play against the engine or in local two-player mode
- Configurable engine depth (1 - 20)
- Drag and drop or click to move pieces
- See the engine's evaluation on an eval bar (only when playing against it)
## Engine Features
- Minimax for move search and selection
- Alpha-Beta Pruning
- Hand-crafted heuristics-based evaluation (HCE)
- Move Sorting
- Iterative Deepening
- Transposition Table using Zobrist Hashing
- Killer Move heuristics
- Null Move Pruning
- Late Move Reductions (LMR)
- Aspiration windows
- Piece-Square Table for uncrowned pieces
- Repetition detection
## Planned
- LAN multiplayer