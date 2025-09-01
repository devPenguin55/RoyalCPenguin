#ifndef BOOK_H
#define BOOK_H

#include <stdint.h>
#include "board.h"
#include "zobrist.h"

typedef struct PositionHashToMoves {
    uint64_t positionHash;
    int amtPossibleMoves;
    Move possibleMoves[64];
} PositionHashToMoves;

typedef struct OpeningBook {
    PositionHashToMoves *collection;
    int capacity;
    int amount; 
} OpeningBook;

OpeningBook initBook(Board *board, TranspositionTable *tt);
Move bookLookup(Board *board, OpeningBook *book);

#endif 