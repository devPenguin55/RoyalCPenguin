#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <raylib.h>
#include "board.h"
#include "movegen.h"
#include "search.h"
#include "evaluation.h"
#include "moveOrdering.h"
#include "zobrist.h"
#include "book.h"

// #define MAX(a, b) ((a) > (b) ? (a) : (b))



double getTimeInMilliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int POSITIONS_EVALUATED;

int SearchAllCaptures(Board *board, int alpha, int beta, TranspositionTable *tt, SearchRootResult *rootResult) {
    // quiescence search
    POSITIONS_EVALUATED++;


    uint64_t stateZobristHash = board->zobristHash;
    for (int i = (board->moves.size-board->halfmoveClock); i < board->moves.size; i++)
    {
        if (board->moves.stack[i].oldZobristHash == stateZobristHash)
        {
                
             board->gameState = DRAW;
             return 0;
        }
    }
    if (board->halfmoveClock >= 100) {
        board->gameState = DRAW;
        return 0;
    }

    int evaluation = Evaluate(board);
    
    if (evaluation >= beta) {
        return beta;
    }

    if (evaluation > alpha) {
        alpha = evaluation;
    }
    
    LegalMovesContainer legalMoves = generateLegalMoves(board);

    orderMoves(board, &legalMoves, tt, board->targetPly, rootResult);

    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        if (legalMoves.moves[i].captureSquare.type == NONE) { continue; }

        pushMove(board, legalMoves.moves[i]);
        evaluation = -SearchAllCaptures(board, -beta, -alpha, tt, rootResult);
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

int Search(Board *board, int depth, int alpha, int beta, TranspositionTable *tt, SearchRootResult *rootResult) {
    uint64_t stateZobristHash = board->zobristHash;
    if (depth == 0) {
        int evaluation = SearchAllCaptures(board, alpha, beta, tt, rootResult);
        ttStore(tt, stateZobristHash, depth, (board->targetPly - depth), evaluation, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
        return evaluation;
    }
    
    LegalMovesContainer legalMoves = generateLegalMoves(board);

    if (board->gameState == DRAW_RETURN_0_IN_SEARCH) {
        free(legalMoves.moves);
        return 0;
    }

    if (board->gameState > CHECK) {
        // Game Over -> checkmate or stalemate
        free(legalMoves.moves);
        if (board->gameState == CHECKMATE) {
            // * the mate score is infinity value - depthSearched, depth searched is the target ply - depthRemaining
            long long mateScore = -(infinity - (board->targetPly - depth));
            ttStore(tt, stateZobristHash, depth, (board->targetPly - depth), mateScore, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
            return mateScore;
        }
        ttStore(tt, stateZobristHash, depth, (board->targetPly - depth), 0, FLAG_EXACT, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1});
        return 0;
    }

    
    int flag = FLAG_ALPHA;
    
    long long ttLookupValue = ttLookup(tt, stateZobristHash, depth, (board->targetPly - depth), alpha, beta);
    if (ttLookupValue != UNKNOWN) {
        (tt->hits)++;
        if ((depth == board->targetPly) && (ttLookupValue > rootResult->bestScore)) {
            uint64_t key = stateZobristHash;
            rootResult->bestMove = tt->entries[key % tt->size].bestMove;
            rootResult->bestScore = ttLookupValue;
        }
        // printf("tt lookup!\n");
        return ttLookupValue;
    }

    orderMoves(board, &legalMoves, tt, (board->targetPly - depth), rootResult);

    Move bestMoveInSearch = (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1};

    int evaluation;
    int foundPv = 0;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        if (foundPv) {
            evaluation = -Search(board, depth-1, -alpha-1, -alpha, tt, rootResult);
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
                
                     5,177,664 positions evaluated before using on a test position
                     4,990,554 positions evaluated after using on the same test position
                */ 
               evaluation = -Search(board, depth - 1, -beta, -alpha, tt, rootResult);
               foundPv = 0;
            }
        } else {
            evaluation = -Search(board, depth - 1, -beta, -alpha, tt, rootResult);
        }

        
        popMove(board);

        if (evaluation >= beta) {
            // opponent would never allow for this to happen, the move was too good for the the current side
            int curPly = (board->targetPly - depth);
            ttStore(tt, stateZobristHash, depth, curPly, beta, FLAG_BETA, legalMoves.moves[i]);

            if ((legalMoves.moves[i].captureSquare.type != NONE) || (legalMoves.moves[i].promotionType != NONE)) {
                if (rootResult->killers[curPly][0].fromSquare == rootResult->killers[curPly][0].toSquare) {
                    // store the killer in the first slot
                    memcpy(&rootResult->killers[curPly][0], &legalMoves.moves[i], sizeof(Move));
                } else if (rootResult->killers[curPly][1].fromSquare == rootResult->killers[curPly][1].toSquare) {
                    // store the killer in the second slot
                    memcpy(&rootResult->killers[curPly][1], &legalMoves.moves[i], sizeof(Move));                
                }
            }
            free(legalMoves.moves);
            return beta;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
            foundPv = 1;
            flag = FLAG_EXACT;
            bestMoveInSearch = legalMoves.moves[i];
            if ((depth == board->targetPly)) {
                rootResult->bestMove = legalMoves.moves[i];
                rootResult->bestScore = alpha;
            }
        }
        
    }   

    ttStore(tt, stateZobristHash, depth, (board->targetPly - depth), alpha, flag, bestMoveInSearch);
    free(legalMoves.moves);


    return alpha;
}

void convertPieceTypeToTextureColumn2(int pieceType, int *textureCol)
{
    switch (pieceType)
    {
    case KING:
        *textureCol = 0;
        break;
        case QUEEN:
        *textureCol = 1;
        break;
        case BISHOP:
        *textureCol = 2;
        break;
        case KNIGHT:
        *textureCol = 3;
        break;
        case ROOK:
        *textureCol = 4;
        break;
        case PAWN:
        *textureCol = 5;
        break;
    }
}


SearchRootResult IterativeDeepening(Board *board, int maxDepth, TranspositionTable *tt, Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Vector2 *mousePosition, int *textureCol, OpeningBook *book) {
    // ! if u see depth 1 mate found but score is like -999996 or smt, then it found mate but it was in tp and thus cancelled search there 
    // ! bc it knew that it was game over
    
    int searchStartTime = getTimeInMilliseconds();
    POSITIONS_EVALUATED = 0;
    tt->hits = 0;
    tt->collisions = 0;
    tt->writes = 0;
    SearchRootResult rootResult;
    int maxDepthSearched = 1;
    
    
    if (board->gameState > CHECK) {
        return rootResult;
    }

    Move bookMoveResult = bookLookup(board, book);
    if (bookMoveResult.fromSquare != -1) {
        rootResult.bestScore = UNKNOWN;
        rootResult.bestMove = bookMoveResult;
        
        return rootResult;
    }

    if ((board->whitePieceAmt + board->blackPieceAmt) <= 16) {
        maxDepth += (int)((16 - board->whitePieceAmt - board->blackPieceAmt)/(2));
    }

    
    printf("Searching for total depth %d with game state of %d\n", maxDepth, board->gameState);
    
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++) {
        board->targetPly = currentDepth;
        rootResult.bestScore = -infinity;
        Search(board, currentDepth, -infinity, infinity, tt, &rootResult);

        // * graphics addition 
        BeginDrawing();

        // convertPieceTypeToTextureColumn2(drawingPieceMouseHandler->squareSelected.type, &textureCol);

        Rectangle destRec = {
            mousePosition->x - spriteRecs[(*textureCol) + drawingPieceMouseHandler->squareSelected.color * 6].width * 0.75 * 0.5 * 0.5, mousePosition->y - spriteRecs[(*textureCol) + drawingPieceMouseHandler->squareSelected.color * 6].height * 0.75 * 0.5 * 0.5,
            spriteRecs[0].width * 0.75 * 0.5, spriteRecs[0].height * 0.75 * 0.5};

        DrawTexturePro(
            *spriteSheet,
            spriteRecs[(*textureCol) + drawingPieceMouseHandler->squareSelected.color * 6],
            destRec,
            (Vector2){0, 0},
            0.0f,
            WHITE);

        DrawRectangle(8 * 75, 0, 75*4, 75*8, DARKGRAY);
        char text[80];
        if (((rootResult.bestScore > infinity-500) || (rootResult.bestScore < -infinity+500)) && rootResult.bestScore != UNKNOWN) { 
            int whiteCenteredMateScore = (board->colorToPlay == BLACK_PIECE ? -1 : 1)*(infinity-abs(rootResult.bestScore));
            snprintf(text, sizeof(text), "Searched depth %d\nEval:%s#%d", currentDepth, (whiteCenteredMateScore >= 0) ? " \0" : " -\0", (abs(whiteCenteredMateScore)+1)/2);
        } else {
            double whiteCenteredEval = (board->colorToPlay == BLACK_PIECE ? -1 : 1)*((double)(rootResult.bestScore))/((double)(pieceTypeToWorth[PAWN]));
            if (whiteCenteredEval == 0.0) {
                snprintf(text, sizeof(text), "Searched depth %d\nEval: 0.0", currentDepth);
            } else {
                snprintf(text, sizeof(text), "Searched depth %d\nEval:%s%.1f", currentDepth, (whiteCenteredEval >= 0) ? " +\0" : " \0", whiteCenteredEval);
            }
        }
        DrawText(text, 8 * 75+2, 2 * 75, 30, GREEN);
        char notation[5];
        moveToNotation(&(rootResult.bestMove), notation);
        DrawText("Bot Best Move\n---------------", 8 * 75+2, 4 * 75, 30, GREEN);
        DrawText(notation, 8 * 75+2, 5 * 75-15, 30, GREEN);
        EndDrawing();
        
        
        
        maxDepthSearched = currentDepth;
        if (((rootResult.bestScore > infinity-500) || (rootResult.bestScore < -infinity+500)) && rootResult.bestScore != UNKNOWN) {
            // we have a mate score to deal with
            printf("\n[MATE FOUND]");
            break;
        } else {
            printf("depth %d | best move %d to %d, %d was best score\n", currentDepth, rootResult.bestMove.fromSquare, rootResult.bestMove.toSquare, rootResult.bestScore);
        }
        // if (((getTimeInMilliseconds() - searchStartTime)/1000) > 5) {
            //     printf("\n[TIME OUT]");
            //     break;
            // }
        }   
        
        printf("\nTook %f ms for depth %d, %d positions evaluated, %d hits, %d collisions\n", getTimeInMilliseconds()-searchStartTime, maxDepthSearched, POSITIONS_EVALUATED, tt->hits, tt->collisions);
        printf("Wrote %d, which is %f\n\n", tt->writes, ((double)(tt->writes))/((double)(tt->size))*100.0);
        // extract PV line 
    //     printf("\nPV line -> ");
    //     int amtPV = 0;
    //     for (int i = 0; i<maxDepthSearched; i++) {
    //             uint64_t key = board->zobristHash;
    //             Move *bestMoveFromTT = NULL;
    //             TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory
    //             if (pEntry->key == key) {
    //                     // we have a match for the best move that was saved in the TT, will rank it highly
    //                     bestMoveFromTT = &(pEntry->bestMove);
    //                 } 
    //                 if (bestMoveFromTT != NULL && board->squares[bestMoveFromTT->fromSquare].type != NONE) {
    //                         char notation[5];
    //                         moveToNotation(&(*bestMoveFromTT), notation);
    //                         printf("%s ", notation);
    //                         pushMove(board, *bestMoveFromTT);
    //                         amtPV++;
    //                     } else {
    //                             printf("PV extraction failure...");
    //                             break;
    //                         }
    //                     }
    // for (int i = 0; i<amtPV; i++) {
    //     popMove(board);
    // }          

    return rootResult;
}