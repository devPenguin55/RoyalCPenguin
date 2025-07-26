#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "movegen.h"
#include "board.h"

typedef struct ScoredMove
{
    Move move;
    int score;
} ScoredMove;

void orderMoves(Board *board, LegalMovesContainer *legalMoves);


#endif
