#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "board.h"
#include "movegen.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

const int PawnsStart[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    50,  50,  50,  50,  50,  50,  50,  50,
    10,  10,  20,  30,  30,  20,  10,  10,
    5,   5,  10,  25,  25,  10,   5,   5,
    0,   0,   0,  20,  20,   0,   0,   0,
    5,  -5, -10,   0,   0, -10,  -5,   5,
    5,  10,  10, -20, -20,  10,  10,   5,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int PawnsEnd[] = {
	 0,   0,   0,   0,   0,   0,   0,   0,
    80,  80,  80,  80,  80,  80,  80,  80,
    50,  50,  50,  50,  50,  50,  50,  50,
    30,  30,  30,  30,  30,  30,  30,  30,
    20,  20,  20,  20,  20,  20,  20,  20,
    10,  10,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int Rooks[] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

const int Knights[] = {
	-50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

const int Bishops[] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

const int Queens[] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,   0,  5,  5,  5,  5,  0, -5,
    0,    0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int KingStart[] = {
	-80, -70, -70, -70, -70, -70, -70, -80, 
	-60, -60, -60, -60, -60, -60, -60, -60, 
	-40, -50, -50, -60, -60, -50, -50, -40, 
	-30, -40, -40, -50, -50, -40, -40, -30, 
	-20, -30, -30, -40, -40, -30, -30, -20, 
	-10, -20, -20, -20, -20, -20, -20, -10, 
	20,  20,  -5,  -5,  -5,  -5,  20,  20, 
	20,  30,  10,   0,   0,  10,  30,  20
};

const int KingEnd[] = {
	-20, -10, -10, -10, -10, -10, -10, -20,
	-5,   0,   5,   5,   5,   5,   0,  -5,
	-10, -5,   20,  30,  30,  20,  -5, -10,
	-15, -10,  35,  45,  45,  35, -10, -15,
	-20, -15,  30,  40,  40,  30, -15, -20,
	-25, -20,  20,  25,  25,  20, -20, -25,
	-30, -25,   0,   0,   0,   0, -25, -30,
	-50, -30, -30, -30, -30, -30, -30, -50
};

const int *TableLookup[6][2] = {
    {PawnsStart, PawnsEnd},
    {Knights, NULL},
    {Bishops, NULL},
    {Rooks, NULL},
    {Queens, NULL},
    {KingStart, KingEnd},
};

// the static evaluation should be done for both the white and black pieces
// then, the difference should be calculated and the final eval is multiplied by -1 if black to play
double Evaluate(Board *board) {
    int whiteEval = 0;
    int blackEval = 0;
    int whitePositionScore = 0;
    int blackPositionScore = 0;
    
    // int friendlyKingSquareIndex;
    // int opponentKingSquareIndex;

    for (int i = 0; i < board->whitePieceAmt; i++)
    {  
        whiteEval += pieceTypeToWorth[board->whitePieceSquares[i].type];
        whitePositionScore += TableLookup[board->whitePieceSquares[i].type - 1][0][board->whitePieceSquares[i].squareIndex];
        // if (board->whitePieceSquares[i].type == KING) {
        //     if (board->colorToPlay == WHITE_PIECE) {
        //         friendlyKingSquareIndex = board->whitePieceSquares[i].squareIndex;
        //     } else {
        //         opponentKingSquareIndex = board->whitePieceSquares[i].squareIndex;
        //     }
        // }        
    }
    int blackAdjustedIndex;
    for (int i = 0; i < board->blackPieceAmt; i++)
    {  
        blackEval += pieceTypeToWorth[board->blackPieceSquares[i].type];

        blackAdjustedIndex = (7 - (board->blackPieceSquares[i].squareIndex / 8))*8 + (board->blackPieceSquares[i].squareIndex % 8);
        blackPositionScore += TableLookup[board->blackPieceSquares[i].type - 1][0][blackAdjustedIndex];

        // if (board->blackPieceSquares[i].type == KING) {
        //     if (board->colorToPlay == WHITE_PIECE) {
        //         opponentKingSquareIndex = board->blackPieceSquares[i].squareIndex;
        //     } else {
        //         friendlyKingSquareIndex = board->blackPieceSquares[i].squareIndex;
        //     }
        // } 
    }

    double eval = (double)(whiteEval - blackEval) + (((double)(whitePositionScore - blackPositionScore)) / 80.0);

    if (board->colorToPlay == BLACK_PIECE) {
        eval *= -1;
    }

    return eval;    
}