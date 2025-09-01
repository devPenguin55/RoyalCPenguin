#ifndef BOOK_H
#define BOOK_H

#include <stdint.h>
#include "board.h"
#include "zobrist.h"

typedef struct PositionHashToMoves {
    uint64_t positionHash;
    int amtPossibleMoves;
    Move possibleMoves[64];
    int moveOccurrence[64];
} PositionHashToMoves;

typedef struct OpeningBook {
    PositionHashToMoves *collection;
    int capacity;
    int amount; 
} OpeningBook;

typedef struct AllPossibleOpeningMovesFromPosition {
    Move moves[64];
    int moveOccurrence[64];
    int amtMoves;
} AllPossibleOpeningMovesFromPosition;

OpeningBook initBook(Board *board, TranspositionTable *tt);
Move bookLookup(Board *board, OpeningBook *book);
AllPossibleOpeningMovesFromPosition bookAllPossibleMoves(Board *board, OpeningBook *book);

#endif 