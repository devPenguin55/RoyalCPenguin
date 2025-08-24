#ifndef ZOBRIST_H
#define ZOBRIST_H
#include <time.h>
#include "board.h"
#include <stdint.h>
#include "movegen.h"

typedef struct TranspositionTableEntry {
    uint64_t key;
    int depth;
    int flag;
    long long value;
    Move bestMove;
} TranspositionTableEntry;

typedef struct TranspositionTable {
    TranspositionTableEntry *entries;
    int size;
    int writes;
    int hits;
    int collisions;
} TranspositionTable;

uint64_t getRandom64();
void initializeTT(TranspositionTable *tt, int ttMBsToAllocate);
uint64_t generateZobristHash(Board *board);
void ttStore(TranspositionTable *tt, uint64_t key, int depth, int alreadySearchedDepth, long long val, int flag, Move bestMove);
long long ttLookup(TranspositionTable *tt, uint64_t key, int depth, int alreadySearchedDepth, int alpha, int beta);

#endif