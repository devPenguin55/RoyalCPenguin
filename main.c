#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "graphics.h"
#include "movegen.h"
#include "search.h"
#include "evaluation.h"




int main() {
    initMoveGen();
    Board board;


    initBoard(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // initBoard(&board, "3k4/R7/8/4K3/8/8/8/8 w - - 0 1");


    initGraphics(&spriteSheet, spriteRecs, sounds);

    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);

    
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 0, &curLegalMoves);
    }

    free(board.moves.stack);
    free(curLegalMoves.moves);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}