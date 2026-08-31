#pragma once
// Masks used for operations with bitboards for the bot's logic
extern unsigned int squares[32];
extern unsigned int MASK_L5, MASK_L3, MASK_R5, MASK_R3;

void InitBitboardMasks();