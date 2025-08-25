#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "movegen.h"
#include "board.h"

typedef struct ScoredMove
{
    Move move;
    int score;
} ScoredMove;

struct TranspositionTable;
struct SearchRootResult {
    Move bestMove;
    int bestScore;
    Move killers[100][2];
};

void orderMoves(Board *board, LegalMovesContainer *legalMoves, struct TranspositionTable *tt, int ply, struct SearchRootResult *rootResult);


#endif