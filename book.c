#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "book.h"
#include "zobrist.h"


OpeningBook initBook(Board *board, TranspositionTable *tt) {
    OpeningBook book;

    book.capacity = 64;
    book.amount = 0;
    book.collection = malloc(sizeof(PositionHashToMoves)*book.capacity);

    FILE *filePtr = fopen("book.txt", "r");

    if (filePtr == NULL) {
        printf("Could not open book.txt!");
        exit(EXIT_FAILURE);
    }
    

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), filePtr) != NULL) {
        
        if (book.amount >= book.capacity) {
            book.capacity *= 2;
            book.collection = realloc(book.collection, sizeof(PositionHashToMoves)*book.capacity);
            if (book.collection == NULL)
            {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }
        }

        // printf("%s", buffer);

        char fen[1024];
        int endLookForFen = 0;
        int lastFenIndex = 0;

        char encodedMoves[64][8];
        int amtEncodedMoves = 0;
        char curEncodedMove[8];
        int posInCurEncodedMove = 0;
        int keepLookingForEncodedMove = 1;
        char totalCharsForOccurrence[16];
        int amtWrittenToTotalCharsForOccurrence = 0;
        book.collection[book.amount].amtPossibleMoves = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            if ((!endLookForFen) && (buffer[i] != ':')) {
                lastFenIndex = i;
                fen[i] = buffer[i];
            } else {
                endLookForFen = 1;

                if (buffer[i] != ' ' && buffer[i] != ':') {
                    if (buffer[i] == ',') {
                        curEncodedMove[posInCurEncodedMove++] = '\0';
                        strcpy(encodedMoves[amtEncodedMoves++], curEncodedMove);
                        keepLookingForEncodedMove = 1;
                        posInCurEncodedMove = 0;
                        totalCharsForOccurrence[amtWrittenToTotalCharsForOccurrence++] = '\0';
                        amtWrittenToTotalCharsForOccurrence = 0;
                        book.collection[book.amount].moveOccurrence[book.collection[book.amount].amtPossibleMoves++] = atoi(totalCharsForOccurrence);
                    } else if (buffer[i] == '(' || buffer[i] == ')') {
                        keepLookingForEncodedMove = 0;
                    } else if (keepLookingForEncodedMove) {
                        curEncodedMove[posInCurEncodedMove++] = buffer[i];
                    }
                    if (keepLookingForEncodedMove == 0 && !((buffer[i] == '(' || buffer[i] == ')'))) {
                        totalCharsForOccurrence[amtWrittenToTotalCharsForOccurrence++] = buffer[i];
                    }
                } 
            }
        }
        
        totalCharsForOccurrence[amtWrittenToTotalCharsForOccurrence++] = '\0';
        book.collection[book.amount].moveOccurrence[book.collection[book.amount].amtPossibleMoves++] = atoi(totalCharsForOccurrence);
        
        fen[lastFenIndex+1] = '\0';

        curEncodedMove[posInCurEncodedMove++] = '\0';
        strcpy(encodedMoves[amtEncodedMoves++], curEncodedMove);

        // printf("%s\n", fen);
        initBoard(board, fen, tt);
        

        // printf("%lld\n\n", board->zobristHash);

        book.collection[book.amount].positionHash = board->zobristHash;
        book.collection[book.amount].amtPossibleMoves = 0;
        LegalMovesContainer curLegalMoves = generateLegalMoves(board);
        for (int encodedMoveIndex = 0; encodedMoveIndex < amtEncodedMoves; encodedMoveIndex++) {
            int move = atoi(encodedMoves[encodedMoveIndex]);
            int from = move & 0x3F;         
            int to = (move >> 6) & 0x3F;  
            // * don't need promotion here (no grandmaster would be able to obtain/allow a promotion within the first 16 moves)
            int promotion = (move >> 12) & 0xF; 
    
            // * convert to my version of board indexes
            from ^= 56;
            to ^= 56;

            // printf("\n%d | %d %d %d\n", move, from, to, promotion);

            for (int i = 0; i < curLegalMoves.amtOfMoves; i++)
            {
                if (
                    curLegalMoves.moves[i].toSquare == to &&
                    curLegalMoves.moves[i].fromSquare == from)
                {
                    if (curLegalMoves.moves[i].promotionType > PAWN)
                    {
                        if (curLegalMoves.moves[i].promotionType != (promotion+1))
                        {
                            continue;
                        }
                    }
                    book.collection[book.amount].possibleMoves[(book.collection[book.amount].amtPossibleMoves)++] = curLegalMoves.moves[i];
                    break;
                }
            }
            
        }
        free(curLegalMoves.moves);
        free(board->moves.stack); 
        book.amount++;
    }
    fclose(filePtr);

    return book;
}


Move bookLookup(Board *board, OpeningBook *book) {
    for (int i = 0; i < book->amount; i++) {
        if (book->collection[i].positionHash == board->zobristHash) {
            // * we have a match! now return one of the stored moves
            return book->collection[i].possibleMoves[rand() % (book->collection[i].amtPossibleMoves)]; 
        }
    }
    return (Move){-1, -1, -1, (Square){NONE, NONE, -1}, -1};
}

AllPossibleOpeningMovesFromPosition bookAllPossibleMoves(Board *board, OpeningBook *book) {
    AllPossibleOpeningMovesFromPosition possibleBook;
    possibleBook.amtMoves = 0;


    for (int i = 0; i < book->amount; i++) {
        if (book->collection[i].positionHash == board->zobristHash) {
            // * we have a match! now return all the stored moves
            for (int moveIdx = 0; moveIdx<book->collection->amtPossibleMoves; moveIdx++) {
                possibleBook.moves[moveIdx] = book->collection[i].possibleMoves[moveIdx];
                possibleBook.moveOccurrence[moveIdx] = book->collection[i].moveOccurrence[moveIdx];
                possibleBook.amtMoves++;
            }
            break;
        }
    }
    return possibleBook;
}