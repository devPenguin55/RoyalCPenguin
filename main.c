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

int main() {
    Board board;
    
    InitWindow(8*100*0.75, 8*100*0.75, "Chess Board");
    InitAudioDevice();

    
    initBoard(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // initBoard(&board, "r1bqk2r/ppppn1pp/2n2p2/2b1p3/2B1PP2/7N/PPPP2PP/RNBQK2R w KQkq - 0 1"); // no castle
    // initBoard(&board, "r1bqk2r/ppppn1pp/2n5/3bp3/2B2P2/6PN/PPPP3P/RNBQK2R w KQkq - 0 1");
    // initBoard(&board, "r3k2r/pppp2pp/2n5/4p3/2B1bP2/6PN/P2P3P/R3K2R w KQkq - 0 1");
    // initBoard(&board, "r1bqk2r/ppppn1pp/2n5/4p3/2B2P2/3Pb1PN/P6P/R3K2R w KQkq - 0 1"); // no castle


    // for (int i = 0; i<42; i++) {
    //     LegalMovesContainer curLegalMoves = generateLegalMoves(&board);
    //     pushMove(&board, curLegalMoves.moves[0]);
    // }
    // printf("push move");    
    
    initGraphics(&spriteSheet, spriteRecs, sounds);
    initMoveGen();

    int verifiedPerft[9] = {1, 20, 400, 8902, 197281, 4865609, 119060324, 3195901860, 84998978956}; // from depth 0 to 8

    for (int depth = 0; depth<= 6; depth++) {
        printf("\nStarting the perft test for depth %d", depth);
        clock_t start = clock();
        int perft = moveGenerationTest(&board, depth);
        printf("\n%d moves generated %s\n", perft, (perft == verifiedPerft[depth]) ? "PASS" : "FAIL");
        clock_t end = clock();
        printf("%fms for depth %d\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0, depth);
    }


    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);
    
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 1, &curLegalMoves);
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