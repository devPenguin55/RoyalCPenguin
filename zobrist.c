#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "zobrist.h"

uint64_t getRandom64() {
    return ((uint64_t)rand() << 48) ^
           ((uint64_t)rand() << 32) ^
           ((uint64_t)rand() << 16) ^
           ((uint64_t)rand());
}

uint64_t zobristUniqueValues[70][12]; 

void initializeTT(TranspositionTable *tt, int ttMBsToAllocate) {
    srand(55);
    
    int currentValueIndex;
    
    
    // 64 squares, for each square, first 6 = white, last 6 = black
    for (int i = 0; i < 64; i++)
    {   
        for (int color = WHITE_PIECE; color <= BLACK_PIECE; color++) {
            currentValueIndex = color*6;
            for (int pieceType = PAWN; pieceType <= KING; pieceType++)
            {   
                zobristUniqueValues[i][currentValueIndex] = getRandom64();
                currentValueIndex++;
            }
        }
    }

    for (int i = 64; i<=69; i++) {
        zobristUniqueValues[i][0] = getRandom64();
    }
    
    // convert MB to amount of entries
    int bytesToAllocate = (ttMBsToAllocate * 1024 * 1024) ; // 1 megabyte = 1024 kilobytes = 1024*1024 bytes
    // figure out how many TT entries can fit in these allocated bytes
    tt->size = bytesToAllocate / sizeof(TranspositionTableEntry);
    // then allocate the amount of TT entries that can fit multiplied by the size of a TT entry
    tt->entries = malloc(tt->size * sizeof(TranspositionTableEntry));
    if (tt->entries == NULL) {
        fprintf(stderr, "\nFailed to allocate %d MB for the TT table -> space for %d entries\n", ttMBsToAllocate, tt->size);
        exit(EXIT_FAILURE);
    }
    tt->hits = 0;
    tt->collisions = 0;

    printf("\nCreated TT table with %d MB -> space for %d entries\n", ttMBsToAllocate, tt->size);
}

uint64_t generateZobristHash(Board *board) {
    uint64_t hash = 0;
    for (int i = 0; i < board->whitePieceAmt; i++)
    {
        hash ^= zobristUniqueValues[board->whitePieceSquares[i].squareIndex][board->whitePieceSquares[i].type - PAWN];
    }
    for (int i = 0; i < board->blackPieceAmt; i++)
    {
        hash ^= zobristUniqueValues[board->blackPieceSquares[i].squareIndex][board->blackPieceSquares[i].type - PAWN + 6];
    }

    if (board->colorToPlay == BLACK_PIECE) {
        hash ^= zobristUniqueValues[64][0]; // XOR for black turn
    }

    for (int i = 0; i < 4; i++)
    {   
        if (board->castlingRights[i]) {
            hash ^= zobristUniqueValues[65+i][0];
        }
    }
    
    if (board->enPassantSquareIndex != -1) {
        hash ^= zobristUniqueValues[70][0];
    }


    return hash;
}


long long ttLookup(TranspositionTable *tt, uint64_t key, int depth, int alreadySearchedDepth, int alpha, int beta){
    TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory

    if (pEntry->key == key) {
        if (pEntry->depth >= depth) {
            long long val = pEntry->value;
            if (((val > infinity-500) || (val < -infinity+500)) && val != UNKNOWN) {
                // we have a mate score to deal with, restore it to the current depth already searched (ply)
                int sign = (val > 0) - (val < 0);

                val = ((sign * val) - alreadySearchedDepth) * sign;
            }
            if (pEntry->flag == FLAG_EXACT) {
                return val;
            } else if (pEntry->flag == FLAG_ALPHA && val <= alpha) {
                return alpha;
            } else if (pEntry->flag == FLAG_BETA && val >= beta) {
                return beta;
            }
        }
    }

    return UNKNOWN;
}   

void ttStore(TranspositionTable *tt, uint64_t key, int depth, int alreadySearchedDepth, long long val, int flag, Move bestMove) {
    TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory

    if (pEntry->key == key) {
        (tt->collisions)++;

        if (((val > infinity - 500) || (val < -infinity + 500)) && ((pEntry->value > infinity - 500) || (pEntry->value < -infinity + 500))) {
            // if mate score and already stored value is mate score, prefer the one that is shallower and gets mate fastest
            if ((val > 0) && (pEntry->value > 0)) {
                // both positive, take the highest one
                if (val <= (pEntry->value)) {
                    return;
                }
            } else if ((val < 0) && (pEntry->value < 0)) {
                // both negative, take the lowest one
                if (val >= (pEntry->value)) {
                    return;
                }
            } else {
                // conflicting, take the win
                if (val <= pEntry->value) {
                    return;
                }
            }
        }
    } else {
        (tt->writes)++;
    }

    if (((val > infinity-500) || (val < -infinity+500)) && val != UNKNOWN) {
        // we have a mate score to deal with, restore it to the current depth already searched (ply)
        int sign = (val > 0) - (val < 0);
        val = ((sign * val) + alreadySearchedDepth) * sign;
    }

    pEntry->value = val;
    pEntry->key = key;
    pEntry->bestMove = bestMove;
    pEntry->depth = depth;
    pEntry->flag = flag;
}

