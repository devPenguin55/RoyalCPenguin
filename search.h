#ifndef SEARCH_H
#define SEARCH_H
#include "board.h"
#include "zobrist.h"

typedef struct SearchRootResult {
    Move bestMove;
    int bestScore;
} SearchRootResult;

double getTimeInMilliseconds();
int SearchAllCaptures(Board *board, int alpha, int beta, TranspositionTable *tt);
int Search(Board *board, int depth, int alpha, int beta, TranspositionTable *tt, SearchRootResult *rootResult);
Move IterativeDeepening(Board *board, int maxDepth, TranspositionTable *tt);
#endif