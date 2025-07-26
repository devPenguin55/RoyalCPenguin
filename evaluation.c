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
int Evaluate(Board *board) {
    int whiteEval = 0;
    int blackEval = 0;
    
    int friendlyKingSquareIndex;
    int opponentKingSquareIndex;
    
    for (int i = 0; i < board->whitePieceAmt; i++)
    {  
        whiteEval += pieceTypeToWorth[board->whitePieceSquares[i].type];
        if (board->whitePieceSquares[i].type == KING) {
            if (board->colorToPlay == WHITE_PIECE) {
                friendlyKingSquareIndex = board->whitePieceSquares[i].squareIndex;
            } else {
                opponentKingSquareIndex = board->whitePieceSquares[i].squareIndex;
            }
        }        
    }
    for (int i = 0; i < board->blackPieceAmt; i++)
    {  
        blackEval += pieceTypeToWorth[board->blackPieceSquares[i].type];

        if (board->blackPieceSquares[i].type == KING) {
            if (board->colorToPlay == WHITE_PIECE) {
                opponentKingSquareIndex = board->blackPieceSquares[i].squareIndex;
            } else {
                friendlyKingSquareIndex = board->blackPieceSquares[i].squareIndex;
            }
        } 
    }

    int eval = whiteEval - blackEval;

    if (board->colorToPlay == BLACK_PIECE) {
        eval *= -1;
    }

    return eval;    
}