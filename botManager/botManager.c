#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include "board.h"
#include "notations.h"
#include "zobrist.h"

typedef struct RoughlyEqualStartingPositions {
    int amtEntries;
    char entries[1000][150];
} RoughlyEqualStartingPositions;

int sendMessageToEngine(char *message, HANDLE engineWriteHandle) {
    DWORD bytesWritten;
    if (!WriteFile(engineWriteHandle, message, (DWORD)(strlen(message)), &bytesWritten, NULL)) {
        printf("Failed to write to engine\n");
        return 0;
    }   
    printf("Sent msg %s", message);
    return 1;
}

int readMessageFromEngine(HANDLE hReadPipe, char *moveBuffer) {
    DWORD bytesRead = 0;
    char c;
    size_t pos = 0;

    while (pos < 63) {
        if (!ReadFile(hReadPipe, &c, 1, &bytesRead, NULL) || bytesRead == 0)
            continue; 

        if (c == '\n') {
            break; 
        }
        moveBuffer[pos++] = c;
    }

    if (moveBuffer[pos-1] == '\n') {
        moveBuffer[pos] = '\0';  
    } else {
        moveBuffer[pos] = '\n';  
        moveBuffer[pos+1] = '\0';  
    }
    printf("Received msg: %s", moveBuffer);
    return 1;
}

void initStartingPositions(RoughlyEqualStartingPositions *startingPositions) {
    startingPositions->amtEntries = 0;

    FILE *filePtr = fopen("botManagerStartpos.txt", "r");

    if (filePtr == NULL) {
        printf("Failed to open botManagerStartpos.txt");
        exit(EXIT_FAILURE);
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), filePtr) != NULL) {
        int lastFenIndex = 0;

        if (startingPositions->amtEntries >= 1000) {
            break;
        } else if (((rand() % 30) != 0)) { 
            continue; 
        }
        
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] != '\n') {
                startingPositions->entries[startingPositions->amtEntries][i] = buffer[i];
                lastFenIndex = i;
            }
        }
        
        startingPositions->entries[startingPositions->amtEntries][lastFenIndex+1] = '\0';
        startingPositions->amtEntries++;
    }

    fclose(filePtr);
}

int main() {
    srand(55); // time (null)

    RoughlyEqualStartingPositions startingPositions;
    initStartingPositions(&startingPositions);

    initMoveGen();

    HANDLE hReadEngineOne, hWriteEngineOne;
    HANDLE hReadEngineOneOut, hWriteEngineOneOut;
    HANDLE hReadEngineTwo, hWriteEngineTwo;
    HANDLE hReadEngineTwoOut, hWriteEngineTwoOut;
    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
    
    if (!CreatePipe(&hReadEngineOne, &hWriteEngineOne, &sa, 0)) {
        printf("Failed to create pipe for engine one\n");
        return 1;
    }
    if (!CreatePipe(&hReadEngineOneOut, &hWriteEngineOneOut, &sa, 0)) {
        printf("Failed to create pipe for engine one out\n");
        return 1;
    }
    if (!CreatePipe(&hReadEngineTwo, &hWriteEngineTwo, &sa, 0)) {
        printf("Failed to create pipe for engine two\n");
        return 1;
    }
    if (!CreatePipe(&hReadEngineTwoOut, &hWriteEngineTwoOut, &sa, 0)) {
        printf("Failed to create pipe for engine one out\n");
        return 1;
    }

    printf("\nPipes for engine one and two created\n\n");


    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput  = hReadEngineOne;    
    si.hStdOutput = hWriteEngineOneOut; 
    if (!CreateProcess(
        NULL, 
        "..\\v1.0\\versionOne.exe --nogui 0",
        NULL, NULL,      
        TRUE,            
        0,       
        NULL,   
        "..\\v1.0",      
        &si,
        &pi
    )) {
        printf("Failed to launch engine one!");
        return 1;
    }

    STARTUPINFO si2 = { sizeof(si2) };
    PROCESS_INFORMATION pi2;
    si2.dwFlags |= STARTF_USESTDHANDLES;
    si2.hStdInput  = hReadEngineTwo;    
    si2.hStdOutput = hWriteEngineTwoOut; 
    if (!CreateProcess(
        NULL, 
        "..\\v1.0\\versionOne.exe --nogui 1",
        NULL, NULL,      
        TRUE,            
        0,       
        NULL,   
        "..\\v1.0",      
        &si2,
        &pi2
    )) {
        printf("Failed to launch engine two!");
        return 1;
    }

    Board authoritativeBoard;
    TranspositionTable tt;
    initializeTT(&tt, 256);

    int engineOneWins = 0;
    int engineTwoWins = 0;
    int draws = 0;


    


    for (int gameCount = 0; gameCount < 1000; gameCount++) {
        printf("\nGame %d\n", gameCount+1);
        char positionMessage[160];
        char *fen = startingPositions.entries[gameCount];

        snprintf(positionMessage, sizeof(positionMessage), "%s\n", fen);
        sendMessageToEngine(positionMessage, hWriteEngineOne);
        sendMessageToEngine(positionMessage, hWriteEngineTwo);

        char engineOneMove[256];
        char engineTwoMove[256];
        
        initBoard(&authoritativeBoard, fen, &tt);
        
        LegalMovesContainer legal = generateLegalMoves(&authoritativeBoard);
        free(legal.moves);
        
        printf("game state: %d\n", authoritativeBoard.gameState);
        if (authoritativeBoard.gameState > CHECK) {
            printf("Skipping game, game state is %d\n", authoritativeBoard.gameState);
            free(authoritativeBoard.moves.stack);
            sendMessageToEngine("new\n", hWriteEngineOne);
            sendMessageToEngine("new\n", hWriteEngineTwo);
            continue;
        }

        printf("\nTurn %d\n", authoritativeBoard.colorToPlay);
        if (authoritativeBoard.colorToPlay == ((gameCount < 500) ? WHITE_PIECE : BLACK_PIECE)) {
            sendMessageToEngine("go\n", hWriteEngineOne);
            readMessageFromEngine(hReadEngineOneOut, engineOneMove);

            pushUCIToBoard(&authoritativeBoard, engineOneMove);
        } else {
            sendMessageToEngine("go\n", hWriteEngineTwo);
            readMessageFromEngine(hReadEngineTwoOut, engineTwoMove);
            
            pushUCIToBoard(&authoritativeBoard, engineTwoMove);
        }

        LegalMovesContainer newLegal = generateLegalMoves(&authoritativeBoard);
        free(newLegal.moves);


        


        while ((authoritativeBoard.gameState <= CHECK)) {
            if (authoritativeBoard.colorToPlay == ((gameCount < 500) ? WHITE_PIECE : BLACK_PIECE)) {
                printf("engine one turn\n");
                // engine 1 move
                sendMessageToEngine(engineTwoMove, hWriteEngineOne);
                sendMessageToEngine("go\n", hWriteEngineOne);
                readMessageFromEngine(hReadEngineOneOut, engineOneMove);
                
                pushUCIToBoard(&authoritativeBoard, engineOneMove);
            } else {
                printf("engine two turn\n");
                // engine 2 move
                sendMessageToEngine(engineOneMove, hWriteEngineTwo);
                sendMessageToEngine("go\n", hWriteEngineTwo);
                readMessageFromEngine(hReadEngineTwoOut, engineTwoMove);
                
                pushUCIToBoard(&authoritativeBoard, engineTwoMove);
            } 

            LegalMovesContainer legal = generateLegalMoves(&authoritativeBoard);
            free(legal.moves);
        }
        printf("\n\n\n\n\n");
        if (authoritativeBoard.gameState == CHECKMATE) {
            if (authoritativeBoard.colorToPlay == ((gameCount < 500) ? BLACK_PIECE : WHITE_PIECE)) {
                printf("Engine One Won!\n");
                engineOneWins++;
            } else {
                printf("Engine Two Won!\n");
                engineTwoWins++;
            }
        } else if (authoritativeBoard.gameState == DRAW) {
            printf("Draw!\n");
            draws++;
        }
        printf("\n\n\n\n\n");

        free(authoritativeBoard.moves.stack);
        sendMessageToEngine("new\n", hWriteEngineOne);
        sendMessageToEngine("new\n", hWriteEngineTwo);
    }
    free(tt.entries);
    sendMessageToEngine("startpos\n", hWriteEngineOne);
    sendMessageToEngine("startpos\n", hWriteEngineTwo);
    sendMessageToEngine("q\n", hWriteEngineOne);
    sendMessageToEngine("q\n", hWriteEngineTwo);

    printf("Engine One Wins - %d, Draws - %d, Engine Two Wins - %d", engineOneWins, draws, engineTwoWins);
    CloseHandle(hReadEngineOne);
    CloseHandle(hWriteEngineOne);
    CloseHandle(hReadEngineOneOut);
    CloseHandle(hWriteEngineOneOut);
    CloseHandle(hReadEngineTwo);
    CloseHandle(hWriteEngineTwo);
    CloseHandle(hReadEngineTwoOut);
    CloseHandle(hWriteEngineTwoOut);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);
    return 0;
}