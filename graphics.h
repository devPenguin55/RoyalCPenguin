#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <raylib.h>
#include "board.h"
#include "movegen.h"
#include "zobrist.h"

typedef struct DrawingPieceMouseHandler {
    Square squareSelected;
    int isPickedUp;
} DrawingPieceMouseHandler;


extern Sound sounds[6];
extern Texture2D spriteSheet;
extern Rectangle spriteRecs[12];
extern DrawingPieceMouseHandler drawingPieceMouseHandler;

void initGraphics(Texture2D *spriteSheet, Rectangle *spriteRecs, Sound *sounds);
void convertPieceTypeToTextureColumn(int pieceType, int *textureCol);
void drawFrame(Board *board, Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Sound *sounds, int showIndexes, LegalMovesContainer *curLegalMoves, TranspositionTable *tt);


#endif