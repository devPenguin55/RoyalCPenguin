#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "movegen.h"
#include "moveOrdering.h"
#include "zobrist.h"

int compareInDescendingOrder(const void *a, const void *b) {
    // cast the raw pointers to ScoredMove pointers
    const ScoredMove *moveA = (const ScoredMove *)a;
    const ScoredMove *moveB = (const ScoredMove *)b;

    return moveB->score - moveA->score;
}

void orderMoves(Board *board, LegalMovesContainer *legalMoves, struct TranspositionTable *tt) {
    int score;
    int possiblePawnAttackingIndexes[2];
    int pawnAttackingDirection = (board->colorToPlay == WHITE_PIECE) ? -1 : 1;

    ScoredMove scoredMoves[legalMoves->amtOfMoves];

    uint64_t key = generateZobristHash(board);
    Move *bestMoveFromTT = NULL;
    TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory
    if (pEntry->key == key) {
        // we have a match for the best move that was saved in the TT, will rank it highly
        bestMoveFromTT = &(pEntry->bestMove);
    }

    for (int i = 0; i<legalMoves->amtOfMoves; i++) {
        score = 0;
        if (legalMoves->moves[i].captureSquare.type != NONE) {
            score += 10*pieceTypeToWorth[legalMoves->moves[i].captureSquare.type] - pieceTypeToWorth[board->squares[legalMoves->moves[i].fromSquare].type];
        }

        if (legalMoves->moves[i].promotionType != NONE) {
            score += pieceTypeToWorth[legalMoves->moves[i].promotionType];
        }

        possiblePawnAttackingIndexes[0] = legalMoves->moves[i].fromSquare + 9 * pawnAttackingDirection;
        possiblePawnAttackingIndexes[1] = legalMoves->moves[i].fromSquare + 7 * pawnAttackingDirection;

        if (possiblePawnAttackingIndexes[0] >= 0 && possiblePawnAttackingIndexes[0] <= 63) {
            if (board->squares[possiblePawnAttackingIndexes[0]].type == PAWN) {
                score -= board->squares[legalMoves->moves[i].fromSquare].type;
            }
        } else if (possiblePawnAttackingIndexes[1] >= 0 && possiblePawnAttackingIndexes[1] <= 63) {
            if (board->squares[possiblePawnAttackingIndexes[1]].type == PAWN) {
                score -= board->squares[legalMoves->moves[i].fromSquare].type;
            }
        }
        
        if (bestMoveFromTT != NULL) {
            if (
                legalMoves->moves[i].fromSquare == bestMoveFromTT->fromSquare &&
                legalMoves->moves[i].toSquare == bestMoveFromTT->toSquare &&
                legalMoves->moves[i].isEnpassant == bestMoveFromTT->isEnpassant &&
                legalMoves->moves[i].promotionType == bestMoveFromTT->promotionType &&
                legalMoves->moves[i].captureSquare.color == bestMoveFromTT->captureSquare.color &&
                legalMoves->moves[i].captureSquare.type == bestMoveFromTT->captureSquare.type &&
                legalMoves->moves[i].captureSquare.squareIndex == bestMoveFromTT->captureSquare.squareIndex
            ) {
                score += 10000;
            }
        }
        

        scoredMoves[i].score = score;
        scoredMoves[i].move = legalMoves->moves[i];
    }

    qsort(scoredMoves, legalMoves->amtOfMoves, sizeof(ScoredMove), compareInDescendingOrder);

    for (int i = 0; i < legalMoves->amtOfMoves; i++) {
        legalMoves->moves[i] = scoredMoves[i].move;
    }
}