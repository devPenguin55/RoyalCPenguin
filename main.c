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
#include "zobrist.h"



int main() {
    initMoveGen();
    Board board;
    TranspositionTable tt;
    initializeTT(&tt, 256);

    initBoard(&board, STARTING_FEN, &tt);
    // initBoard(&board, "Q7/8/2K5/1N3k1p/2n4P/8/1PP5/8 w - - 1 57", &tt);
    // initBoard(&board, "8/5k1P/1K6/8/8/2P5/1P6/8 w - - 0 1", &tt);

    // initBoard(&board, "1n1q1knr/r1p4p/1p1p2p1/pP1Q2N1/2N1P3/2P5/1P3PP1/R4RK1 b - - 0 1", &tt);

    // initBoard(&board, "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1", &tt); // mate in a whole lot long
    
    // initBoard(&board, "k7/2R5/3K4/8/8/8/8/8 w - - 28 15", &tt); // m3
    // initBoard(&board, "3k4/8/R7/4K3/8/8/8/8 w - - 50 26", &tt); // m6
    // initBoard(&board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &tt); //kiwipete


    
    initGraphics(&spriteSheet, spriteRecs, sounds);
    
    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);

    
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 1, &curLegalMoves, &tt);
    }

    free(tt.entries);
    free(board.moves.stack);
    free(curLegalMoves.moves);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}