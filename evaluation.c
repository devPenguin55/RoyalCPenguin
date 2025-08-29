#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "board.h"
#include "movegen.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// the static evaluation should be done for both the white and black pieces
// then, the difference should be calculated and the final eval is multiplied by -1 if black to play
double Evaluate(Board *board) {
    double eval = (double)(board->materialScore) + (((double)(board->pieceSquareTableScore)) * 0.5);

    if (board->colorToPlay == BLACK_PIECE) {
        eval *= -1;
    }

    return eval;    
}