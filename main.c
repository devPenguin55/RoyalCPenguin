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


    // initBoard(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    initBoard(&board, "r1b1kb1r/pppp1ppp/5q2/4n3/3KP3/2N3PN/PPP4P/R1BQ1B1R b kq - 0 1"); // simple M1

    // initBoard(&board, "3k4/R7/8/4K3/8/8/8/8 w - - 0 1"); // endgame heuristic testing


    initGraphics(&spriteSheet, spriteRecs, sounds);

    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);

    
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 1, &curLegalMoves);
    }

    free(board.moves.stack);
    free(curLegalMoves.moves);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}