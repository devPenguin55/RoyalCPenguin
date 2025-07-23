#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "graphics.h"
#include "movegen.h"


double getTimeInMilliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main() {
    initMoveGen();
    Board board;
    
    
    
    initBoard(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // int verifiedPerft[7] = {1, 20, 400, 8902, 197281, 4865609, 119060324}; 
    // for (int depth = 0; depth<= 6; depth++) {
    //     printf("\nStarting the perft test for depth %d, expecting %d nodes", depth, verifiedPerft[depth]);
    //     double start = getTimeInMilliseconds();
    //     int perft = moveGenerationTest(&board, depth);
    //     printf("\n%d moves generated %s\n", perft, (perft == verifiedPerft[depth]) ? "PASS" : "FAIL");
    //     double end = getTimeInMilliseconds();
    //     printf("%fms for depth %d, %f million nodes/sec\n", end - start, depth, (perft / ((end - start) / 1000.0)) / 1000000);
    // }
    // printf("\nPERFT COMPLETE!\n");

    initGraphics(&spriteSheet, spriteRecs, sounds);

    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);
    
    int aiWaitToMove = 0;
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 1, &curLegalMoves, &aiWaitToMove);
    }

    free(board.moves.stack);
    free(curLegalMoves.moves);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}