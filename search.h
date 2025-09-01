#ifndef SEARCH_H
#define SEARCH_H
#include "board.h"
#include "zobrist.h"
#include "book.h"


typedef struct SearchRootResult {
    Move bestMove;
    int bestScore;
    Move killers[100][2];
    // int history[2][64][64];
} SearchRootResult;

typedef struct DrawingPieceMouseHandler
{
    Square squareSelected;
    int isPickedUp;
} DrawingPieceMouseHandler;
void moveToNotation(Move *move, char *notation);

double getTimeInMilliseconds();
int SearchAllCaptures(Board *board, int alpha, int beta, TranspositionTable *tt, SearchRootResult *rootResult);
int Search(Board *board, int depth, int alpha, int beta, TranspositionTable *tt, SearchRootResult *rootResult);
void convertPieceTypeToTextureColumn2(int pieceType, int *textureCol);
SearchRootResult IterativeDeepening(Board *board, int maxDepth, TranspositionTable *tt,Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Vector2 *mousePosition, int *textureCol, OpeningBook *book, char *notation);
#endif