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
#include "book.h"



int main() {
    initMoveGen();
    Board board;
    TranspositionTable tt;
    initializeTT(&tt, 256);
    
    OpeningBook book = initBook(&board, &tt);
    
    initBoard(&board, STARTING_FEN, &tt);
    // initBoard(&board, "2rq1rk1/1p2npp1/4p1b1/4PBQp/5N2/2R3R1/PP3PPP/6K1 w - - 0 1", &tt);
    // initBoard(&board, "8/5k1P/1K6/8/8/2P5/1P6/8 w - - 0 1", &tt);

    // initBoard(&board, "1n1q1knr/r1p4p/1p1p2p1/pP1Q2N1/2N1P3/2P5/1P3PP1/R4RK1 b - - 0 1", &tt);

    // initBoard(&board, "Q7/8/3P4/2K5/3P4/8/3k4/8 w - - 1 29", &tt); // mate in a whole lot long
    
    // initBoard(&board, "k7/2R5/3K4/8/8/8/8/8 w - - 28 15", &tt); // m3
    // initBoard(&board, "3k4/8/R7/4K3/8/8/8/8 w - - 50 26", &tt); // m6
    // initBoard(&board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &tt); //kiwipete

    // initBoard(&board, "r2r2k1/ppq2ppp/4pn2/2bn2B1/2BN4/8/PPP1QPPP/R2R2K1 b - - 0 1", &tt); //test

    initGraphics(&spriteSheet, spriteRecs, sounds);
    
    LegalMovesContainer curLegalMoves = generateLegalMoves(&board);

    SearchRootResult *result = &((SearchRootResult){(Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1}, Evaluate(&board)});    

    int draggingPieceType = -1;
    while (!WindowShouldClose()) {
        drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 0, &curLegalMoves, &tt, result, &draggingPieceType, &book);
    }

    free(tt.entries);
    free(board.moves.stack);
    free(curLegalMoves.moves);
    free(book.collection);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}