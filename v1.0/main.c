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
#include "notations.h"


int main(int argc, char *argv[]) {
    initMoveGen();
    Board board;
    TranspositionTable tt;
    initializeTT(&tt, 256);
    
    OpeningBook book = initBook(&board, &tt);
    
    
    // initBoard(&board, "2rq1rk1/1p2npp1/4p1b1/4PBQp/5N2/2R3R1/PP3PPP/6K1 w - - 0 1", &tt);
    // initBoard(&board, "8/5k1P/1K6/8/8/2P5/1P6/8 w - - 0 1", &tt);

    // initBoard(&board, "1n1q1knr/r1p4p/1p1p2p1/pP1Q2N1/2N1P3/2P5/1P3PP1/R4RK1 b - - 0 1", &tt);

    // initBoard(&board, "Q7/8/3P4/2K5/3P4/8/3k4/8 w - - 1 29", &tt); // mate in a whole lot long
    
    // initBoard(&board, "k7/2R5/3K4/8/8/8/8/8 w - - 28 15", &tt); // m3
    // initBoard(&board, "3k4/8/R7/4K3/8/8/8/8 w - - 50 26", &tt); // m6
    // initBoard(&board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &tt); //kiwipete

    // initBoard(&board, "r1bqk1nr/1pp2pb1/p5p1/4N2p/4P3/2pB4/PPP2PPP/R1BQ1RK1 w kq - 0 11", &tt); //test
    // initBoard(&board, "6k1/2p1pp1p/1p4p1/8/8/2P1r3/P5PP/1R4K1 w - - 0 1", &tt);
    
    if (argc > 1 && strcmp(argv[1], "--nogui") == 0) {
        // char engineId[4];
        // strcpy(engineId, argv[2]);

        char engineId[8];
        strncpy(engineId, argv[2], sizeof(engineId) - 1);
        engineId[sizeof(engineId) - 1] = '\0';


        char positionFenString[100];
        if (fgets(positionFenString, sizeof(positionFenString), stdin)) {
            positionFenString[strcspn(positionFenString, "\n")] = '\0';
        }
        
        if (strcmp(positionFenString, "startpos") == 0) {
            initBoard(&board, STARTING_FEN, &tt);
        } else {
            initBoard(&board, positionFenString, &tt);
        }

        char fileName[64];
        snprintf(fileName, sizeof(fileName), "engine%s.txt", engineId);
        FILE *filePtr = fopen(fileName, "w");
        fprintf(filePtr, "cleared logs\n\n");
        fclose(filePtr);

        while (true) {
            char uciMove[150];
            if (fgets(uciMove, sizeof(uciMove), stdin)) {
                uciMove[strcspn(uciMove, "\n")] = '\0';

                char fileName[64];
                snprintf(fileName, sizeof(fileName), "engine%s.txt", engineId);
                FILE *filePtr = fopen(fileName, "a");
                fprintf(filePtr, "received %s\n", uciMove);
                fclose(filePtr);

                if (strcmp(uciMove, "q") == 0) {
                    break;
                } else if (strcmp(uciMove, "new") == 0) {
                    char positionFenString[150];
                    if (fgets(positionFenString, sizeof(positionFenString), stdin)) {
                        positionFenString[strcspn(positionFenString, "\n")] = '\0';
                    }
                    free(board.moves.stack);                        

                    free(tt.entries);
                    initializeTT(&tt, 256);

                    char fileName[64];
                    snprintf(fileName, sizeof(fileName), "engine%s.txt", engineId);
                    FILE *filePtr = fopen(fileName, "a");
                    fprintf(filePtr, "new game\n");
                    
                    if (strcmp(positionFenString, "startpos") == 0) {
                        initBoard(&board, STARTING_FEN, &tt);
                    } else {
                        initBoard(&board, positionFenString, &tt);
                    }
                    fprintf(filePtr, "board initialized for the new game\n");
                    fclose(filePtr); 
                } else if (strcmp(uciMove, "go") == 0) {
                    char fileName[64];
                    snprintf(fileName, sizeof(fileName), "engine%s.txt", engineId);
                    FILE *filePtr = fopen(fileName, "a");
                    fprintf(filePtr, "iterative deepening calling now for game state %d and %lld\n", board.gameState, board.zobristHash);
                    fclose(filePtr); 

                    SearchRootResult searchResult = IterativeDeepening(&board, 2, &tt, &book, engineId);

                    snprintf(fileName, sizeof(fileName), "engine%s.txt", engineId);
                    FILE *filePtr2 = fopen(fileName, "a");
                    fprintf(filePtr2, "iterative deepening call over\n");
                    fclose(filePtr2); 

                    pushMove(&board, searchResult.bestMove);
            
                    char uciBestMove[6];
                    convertMoveToUCI(&board, searchResult.bestMove, uciBestMove);
                    printf("%s\n", uciBestMove);


                } else {
                    pushUCIToBoard(&board, uciMove);
                }
                fflush(stdout);
            }
        }

        free(tt.entries);
        free(board.moves.stack);
        free(book.collection);
    } else {
        initBoard(&board, STARTING_FEN, &tt);

        initGraphics(&spriteSheet, spriteRecs, sounds);
        Font myFont = LoadFontEx("KRONIKA_.TTF", 32, 0, 0);
        SetTextureFilter(myFont.texture, TEXTURE_FILTER_BILINEAR);
        
        LegalMovesContainer curLegalMoves = generateLegalMoves(&board);
    
        SearchRootResult *result = &((SearchRootResult){(Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1}, Evaluate(&board)});    
    
        int draggingPieceType = -1;
        char notation[64];
        convertMoveToSAN(&board, (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1}, notation);
        while (!WindowShouldClose()) {
            drawFrame(&board, &spriteSheet, spriteRecs, &drawingPieceMouseHandler, sounds, 0, &curLegalMoves, &tt, result, &draggingPieceType, &book, notation, &myFont);
        }
    
        free(tt.entries);
        free(board.moves.stack);
        free(curLegalMoves.moves);
        free(book.collection);
        CloseAudioDevice();
        CloseWindow();
        UnloadFont(myFont);
    }
    return 0;
}