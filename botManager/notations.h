#ifndef NOTATIONS_H
#define NOTATIONS_H

#include "board.h"

void convertSquareIndexToAlgebraicForm(int squareIndex, char *out);
void convertMoveToSAN(Board *board, Move move, char *out);
void convertMoveToUCI(Board *board, Move move, char *out);
void pushUCIToBoard(Board *board, char *uci);

#endif