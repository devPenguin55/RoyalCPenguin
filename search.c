#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "movegen.h"
#include "search.h"
#include "evaluation.h"
#include "moveOrdering.h"

// #define MAX(a, b) ((a) > (b) ? (a) : (b))
int infinity = 9999999;

double getTimeInMilliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int POSITIONS_EVALUATED;

int SearchAllCaptures(Board *board, int alpha, int beta) {
    // quiescence search
    POSITIONS_EVALUATED++;
    int evaluation = Evaluate(board);
    
    if (evaluation >= beta) {
        return beta;
    }

    if (evaluation > alpha) {
        alpha = evaluation;
    }

    LegalMovesContainer legalMoves = generateLegalMoves(board);

    orderMoves(board, &legalMoves);

    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        if (legalMoves.moves[i].captureSquare.type == NONE) { continue; }

        pushMove(board, legalMoves.moves[i]);
        evaluation = -SearchAllCaptures(board, -beta, -alpha);
        popMove(board);

        if (evaluation >= beta) {
            // opponent would never allow for this to happen, the move was too good for the the current side
            free(legalMoves.moves);
            return beta;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
        }
    }   
    
    free(legalMoves.moves);

    return alpha;
}

int Search(Board *board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return SearchAllCaptures(board, alpha, beta);
    }

    
    LegalMovesContainer legalMoves = generateLegalMoves(board);

    if (board->gameState > CHECK) {
        // Game Over -> checkmate or stalemate
        free(legalMoves.moves);
        if (board->gameState == CHECKMATE) {
            return -infinity;
        }
        return 0;
    }

    orderMoves(board, &legalMoves);

    int evaluation;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        evaluation = -Search(board, depth - 1, -beta, -alpha);
        popMove(board);

        if (evaluation >= beta) {
            // opponent would never allow for this to happen, the move was too good for the the current side
            free(legalMoves.moves);
            return beta;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
        }
    }   

    free(legalMoves.moves);

    return alpha;
}

Move SearchRoot(Board *board, int depth) {    
    LegalMovesContainer legalMoves = generateLegalMoves(board);
    orderMoves(board, &legalMoves);
    

    int bestMoveIndex = -1;    
    POSITIONS_EVALUATED = 0;
    int st = getTimeInMilliseconds();
    
    int evaluation;
    int alpha = -infinity;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        evaluation = -Search(board, depth - 1, -infinity, -alpha);
        popMove(board);

        if (evaluation > alpha) {
            alpha = evaluation;
            bestMoveIndex = i;
        }
    }   
    printf("\n\nTook %f ms for depth %d, %d positions evaluated, move index %d", getTimeInMilliseconds()-st, depth, POSITIONS_EVALUATED, bestMoveIndex);

    Move bestMove = legalMoves.moves[bestMoveIndex];
    free(legalMoves.moves);
    
    return bestMove;
}