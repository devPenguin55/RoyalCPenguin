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

int sendMessageToEngine(char *message, HANDLE engineWriteHandle) {
    DWORD bytesWritten;
    if (!WriteFile(engineWriteHandle, message, (DWORD)(strlen(message)), &bytesWritten, NULL)) {
        printf("Failed to write to engine one\n");
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


int main() {
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
        "..\\v1.0\\versionOne.exe --nogui",
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
        "..\\v1.0\\versionOne.exe --nogui",
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


    sendMessageToEngine("startpos\n", hWriteEngineOne);
    sendMessageToEngine("startpos\n", hWriteEngineTwo);

    char engineOneMove[64];
    char engineTwoMove[64];
    Board authoritativeBoard;
    TranspositionTable tt;
    initMoveGen();
    initializeTT(&tt, 256);
    initBoard(&authoritativeBoard, STARTING_FEN, &tt);

    printf("\nTurn %d\n", authoritativeBoard.colorToPlay);
    if (authoritativeBoard.colorToPlay == WHITE_PIECE) {
        sendMessageToEngine("go\n", hWriteEngineOne);
        readMessageFromEngine(hReadEngineOneOut, engineOneMove);

        pushUCIToBoard(&authoritativeBoard, engineOneMove);
    } else {
        sendMessageToEngine("go\n", hWriteEngineTwo);
        readMessageFromEngine(hReadEngineTwoOut, engineTwoMove);
        
        pushUCIToBoard(&authoritativeBoard, engineTwoMove);
    }
    LegalMovesContainer legal = generateLegalMoves(&authoritativeBoard);
    free(legal.moves);

    while (authoritativeBoard.gameState <= CHECK) {
        if (authoritativeBoard.colorToPlay == WHITE_PIECE) {
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
    if (authoritativeBoard.gameState == CHECKMATE) {
        if (authoritativeBoard.colorToPlay == BLACK_PIECE) {
            printf("Engine One Won as White!\n");
        } else {
            printf("Engine Two Won as Black!\n");
        }
    } else {
        printf("Draw!\n");
    }

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