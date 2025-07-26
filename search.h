#ifndef SEARCH_H
#define SEARCH_H
#include "board.h"

double getTimeInMilliseconds();
int SearchAllCaptures(Board *board, int alpha, int beta);
int Search(Board *board, int depth, int alpha, int beta);
Move SearchRoot(Board *board, int depth);

#endif