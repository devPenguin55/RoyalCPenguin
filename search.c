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
#include "zobrist.h"

// #define MAX(a, b) ((a) > (b) ? (a) : (b))



double getTimeInMilliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int POSITIONS_EVALUATED;

int SearchAllCaptures(Board *board, int alpha, int beta, TranspositionTable *tt) {
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

    orderMoves(board, &legalMoves, tt);

    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        if (legalMoves.moves[i].captureSquare.type == NONE) { continue; }

        pushMove(board, legalMoves.moves[i]);
        evaluation = -SearchAllCaptures(board, -beta, -alpha, tt);
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

int Search(Board *board, int depth, int alpha, int beta, TranspositionTable *tt) {
    if (depth == 0) {
        int evaluation = SearchAllCaptures(board, alpha, beta, tt);
        ttStore(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), evaluation, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
        return evaluation;
    }
    
    LegalMovesContainer legalMoves = generateLegalMoves(board);

    if (board->gameState > CHECK) {
        // Game Over -> checkmate or stalemate
        free(legalMoves.moves);
        if (board->gameState == CHECKMATE) {
            // * the mate score is infinity value - depthSearched, depth searched is the target ply - depthRemaining
            long long mateScore = -(infinity - (board->targetPly - depth));
            ttStore(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), mateScore, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
            return mateScore;
        }
        ttStore(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), 0, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
        return 0;
    }

    
    int flag = FLAG_ALPHA;
    
    long long ttLookupValue = ttLookup(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), alpha, beta);
    if (ttLookupValue != UNKNOWN) {
        (tt->hits)++;
        return ttLookupValue;
    }

    orderMoves(board, &legalMoves, tt);

    Move bestMoveInSearch = (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1};

    int evaluation;
    int foundPv = 0;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        if (foundPv) {
            evaluation = -Search(board, depth-1, -alpha-1, -alpha, tt);
            if ((evaluation > alpha) && (evaluation < beta)) {
                /* 
                check for failure - we search with window (alpha, alpha + 1)
                if we exceeded alpha, then it could be a PV node, but if we end up beating alpha later then it isn't
                also, if we end up having an evaluation less than beta then we have not created a beta cutoff
                     -> if this happens, we must discard the hard work and go back to normal alpha beta with a bigger window 
                
                if this doesn't happen though, the search will skip past the node
                     -> essentially, after finding the PV node that improves alpha, the goal is to check that
                     -> all other moves aren't potentially beating this and warranting the full search
                     -> (the goal is to skip the other nodes and have a performance boost)
                
                     5,177,664
                     4,990,554
                */ 
               evaluation = -Search(board, depth - 1, -beta, -alpha, tt);
               foundPv = 0;
            }
        } else {
            evaluation = -Search(board, depth - 1, -beta, -alpha, tt);
        }
        popMove(board);

        if (evaluation >= beta) {
            // opponent would never allow for this to happen, the move was too good for the the current side
            free(legalMoves.moves);
            ttStore(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), beta, FLAG_BETA, legalMoves.moves[i]);
            return beta;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
            foundPv = 1;
            flag = FLAG_EXACT;
            bestMoveInSearch = legalMoves.moves[i];
        }
        
    }   

    free(legalMoves.moves);

    ttStore(tt, generateZobristHash(board, tt), depth, (board->targetPly - depth), alpha, flag, bestMoveInSearch);

    return alpha;
}

SearchRootResult SearchRoot(Board *board, int depth, TranspositionTable *tt) {    
    board->targetPly = depth;

    LegalMovesContainer legalMoves = generateLegalMoves(board);
    orderMoves(board, &legalMoves, tt);
    

    int bestMoveIndex = -1;    
    
    // int st = getTimeInMilliseconds();
    
    int evaluation;
    int bestScore = -infinity;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        evaluation = -Search(board, depth - 1, -infinity, infinity, tt);
        // printf("\n%d for %d to %d", evaluation, legalMoves.moves[i].fromSquare, legalMoves.moves[i].toSquare);
        popMove(board);

        if (evaluation > bestScore) {
            bestScore = evaluation;
            bestMoveIndex = i;
        }
    }   
    // printf("\nTook %f ms for depth %d, %d positions evaluated, move index %d", getTimeInMilliseconds()-st, depth, POSITIONS_EVALUATED, bestMoveIndex);
    // printf("\nTT had %d hits and %d collisions\n", tt->hits, tt->collisions);
    if (bestScore > infinity-500) {
        // winning mate within 50 moves
        printf("     -> Winning Mate in %d\n", (infinity-bestScore+1)/2);
    } else if (bestScore < -infinity+500) {
        // losing mate within 50 moves
        printf("     -> Losing Mate in %d\n", (infinity+bestScore)/2);
    }

    Move bestMove = legalMoves.moves[bestMoveIndex];
    free(legalMoves.moves);
    
    return (SearchRootResult){bestMove, bestScore};
}

Move IterativeDeepening(Board *board, int maxDepth, TranspositionTable *tt) {
    int searchStartTime = getTimeInMilliseconds();
    POSITIONS_EVALUATED = 0;
    tt->hits = 0;
    tt->collisions = 0;
    tt->writes = 0;
    SearchRootResult rootResult;
    int maxDepthSearched = 1;
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++) {
        rootResult = SearchRoot(board, currentDepth, tt);
        maxDepthSearched = currentDepth;
        if (((rootResult.bestScore > infinity-500) || (rootResult.bestScore < -infinity+500)) && rootResult.bestScore != UNKNOWN) {
            // we have a mate score to deal with
            printf("\n[MATE FOUND]");
            break;
        } else {
            printf("best move %d to %d, %d was best score\n", rootResult.bestMove.fromSquare, rootResult.bestMove.toSquare, rootResult.bestScore);
        }
    }   

    printf("\nTook %f ms for depth %d, %d positions evaluated, %d hits, %d collisions", getTimeInMilliseconds()-searchStartTime, maxDepthSearched, POSITIONS_EVALUATED, tt->hits, tt->collisions);
    
    // // extract PV line
    // for (int i = 0; i<maxDepthSearched; i++) {
    //     uint64_t key = generateZobristHash(board, tt);
    //     Move *bestMoveFromTT = NULL;
    //     TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory
    //     if (pEntry->key == key) {
    //         // we have a match for the best move that was saved in the TT, will rank it highly
    //         bestMoveFromTT = &(pEntry->bestMove);
    //     } 
    //     printf("\n");
    //     if (bestMoveFromTT != NULL && board->squares[bestMoveFromTT->fromSquare].type != NONE) {
    //         printMove(*bestMoveFromTT);
    //         pushMove(board, *bestMoveFromTT);
    //     } else {
    //         printf("PV extraction failure...");
    //         break;
    //     }
    // }
    

    return rootResult.bestMove;
}