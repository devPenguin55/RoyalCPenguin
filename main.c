#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <raylib.h>
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
    
    // initBoard(&board, "rnbqkbnr/p2p1Qpp/1p6/2p1p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 1");
    // initBoard(&board, "7k/5Q2/5K2/8/8/8/8/8 w - - 3 3");

    // initBoard(&board, "1k6/ppp4p/6p1/4pP2/8/2P3P1/PP1PPP1P/1K6 w - - 0 1");
    // initBoard(&board, "rnbqkbnr/pppp1ppp/8/8/3PQ3/8/PPP1PPPP/RNB1KBNR b KQkq - 0 3");
    // initBoard(&board, "r3k3/1p3p2/p2q2p1/bn3P2/1N2PQP1/PB6/3K1R1r/3R4 w - - 0 1");
    // initBoard(&board, "qqqqqqqk/8/6Q1/5Q2/4Q3/3Q4/2Q5/KQ6 w - - 0 1");
    
    // initBoard(&board, "r1bqk2r/ppppn1pp/2n2p2/2b1p3/2B1PP2/7N/PPPP2PP/RNBQK2R w KQkq - 0 1"); // no castle
    // initBoard(&board, "r1bqk2r/ppppn1pp/2n5/3bp3/2B2P2/6PN/PPPP3P/RNBQK2R w KQkq - 0 1");
    // initBoard(&board, "r3k2r/pppp2pp/2n5/4p3/2B1bP2/6PN/P2P3P/R3K2R w KQkq - 0 1");
    // initBoard(&board, "r1bqk2r/ppppn1pp/2n5/4p3/2B2P2/3Pb1PN/P6P/R3K2R w KQkq - 0 1"); // no castle
    // initBoard(&board, "4k3/8/8/8/8/3b4/8/R3K2R w KQ - 0 1"); // no castle
    // initBoard(&board, "r3k2r/8/3B4/8/8/8/8/1K6 b kq - 0 1"); // no castle

        
    

    int verifiedPerft[7] = {1, 20, 400, 8902, 197281, 4865609, 119060324}; // from depth 0 to 6
    for (int depth = 0; depth<= -6; depth++) {
        printf("\nStarting the perft test for depth %d", depth);
        double start = getTimeInMilliseconds();
        int perft = moveGenerationTest(&board, depth);
        printf("\n%d moves generated %s\n", perft, (perft == verifiedPerft[depth]) ? "PASS" : "FAIL");
        double end = getTimeInMilliseconds();
        printf("%fms for depth %d, %f million nodes/sec\n", end - start, depth, (perft / ((end - start) / 1000.0)) / 1000000);
    }
    
    InitWindow(8*100*0.75, 8*100*0.75, "Chess Board");
    InitAudioDevice();
    initGraphics(&spriteSheet, spriteRecs, sounds);

    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);
    
    int aiWaitToMove = 0;
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 1, &curLegalMoves, &aiWaitToMove);
    }

    // printf("\n\n\nmove stack -> \n");
    // for (int i = 0; i < board.moves.size; i++) {
    //     printMove(board.moves.stack[i]);
    // }

    free(board.moves.stack);
    free(curLegalMoves.moves);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}