#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "board.h"
#include "movegen.h"
#include "common.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// the static evaluation should be done for both the white and black pieces
// then, the difference should be calculated and the final eval is multiplied by -1 if black to play
double Evaluate(Board *board) {
    // int whitePositionScore = 0;
    // int blackPositionScore = 0;
    
    // // int friendlyKingSquareIndex;
    // // int opponentKingSquareIndex;

    // for (int i = 0; i < board->whitePieceAmt; i++)
    // {  
    //     whitePositionScore += TableLookup[board->whitePieceSquares[i].type - 1][0][board->whitePieceSquares[i].squareIndex];
    //     // if (board->whitePieceSquares[i].type == KING) {
    //     //     if (board->colorToPlay == WHITE_PIECE) {
    //     //         friendlyKingSquareIndex = board->whitePieceSquares[i].squareIndex;
    //     //     } else {
    //     //         opponentKingSquareIndex = board->whitePieceSquares[i].squareIndex;
    //     //     }
    //     // }        
    // }
    // for (int i = 0; i < board->blackPieceAmt; i++)
    // {  
    //     blackPositionScore += TableLookup[board->blackPieceSquares[i].type - 1][0][(board->blackPieceSquares[i].squareIndex)^56];

    //     // if (board->blackPieceSquares[i].type == KING) {
    //     //     if (board->colorToPlay == WHITE_PIECE) {
    //     //         opponentKingSquareIndex = board->blackPieceSquares[i].squareIndex;
    //     //     } else {
    //     //         friendlyKingSquareIndex = board->blackPieceSquares[i].squareIndex;
    //     //     }
    //     // } 
    // }
    // // printf("original %d\n", whitePositionScore - blackPositionScore);
    // if ((whitePositionScore - blackPositionScore) != board->pieceSquareTableScore) {
    //     printf("MISMATCH original %d | diff %d\n", whitePositionScore - blackPositionScore, board->pieceSquareTableScore);
    //     printBoard(board);
    //     exit(EXIT_FAILURE);
    // }
    double eval = (double)(board->materialScore) + (((double)(board->pieceSquareTableScore)) * 0.5);

    if (board->colorToPlay == BLACK_PIECE) {
        eval *= -1;
    }

    return eval;    
}