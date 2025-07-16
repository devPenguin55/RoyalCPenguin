#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <raylib.h>
#include <math.h>
#include "board.h"
#include "movegen.h"


#define MIN(a, b) ((a) < (b) ? (a) : (b))
const int directionOffsets[8] = {-8, 8, -1, 1, -9, -7, 7, 9};    // up, down, left, right, diagonal upleft, diagonal upright, diagonal downleft, diagonal downright
const int knightOffsets[8] = {-10, -17, -15, -6, 6, 15, 17, 10}; // leftmidup, lefthighup, righthighup, rightmidup, leftmiddown, lefthighdown, righthighdown, rightmiddown
const int kingIndexesToCastle[8] = {61, 62, 59, 58, 5, 6, 3, 2}; //  white kingside(f1, g1) queenside(d1, c1), black kingside(f8, g8) queenside(d8, d8)
    
int numSquaresToEdge[64][20];
void fillNumSquaresToEdge()
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            // normal directions
            numSquaresToEdge[row * 8 + col][0] = row;                   // up
            numSquaresToEdge[row * 8 + col][1] = 7 - row;               // down
            numSquaresToEdge[row * 8 + col][2] = col;                   // left
            numSquaresToEdge[row * 8 + col][3] = 7 - col;               // right
            numSquaresToEdge[row * 8 + col][4] = MIN(row, col);         // up left
            numSquaresToEdge[row * 8 + col][5] = MIN(row, 7 - col);     // up right
            numSquaresToEdge[row * 8 + col][6] = MIN(7 - row, col);     // down left
            numSquaresToEdge[row * 8 + col][7] = MIN(7 - row, 7 - col); // down right

            // knight specific (if the knight can move to a certain location)
            numSquaresToEdge[row * 8 + col][8] = col >= 2 && row >= 1;  // leftmidup
            numSquaresToEdge[row * 8 + col][9] = col >= 1 && row >= 2;  // lefthighup
            numSquaresToEdge[row * 8 + col][10] = col <= 6 && row >= 2; // righthighup
            numSquaresToEdge[row * 8 + col][11] = col <= 5 && row >= 1; // rightmidup
            numSquaresToEdge[row * 8 + col][12] = col >= 2 && row <= 6; // leftmiddown
            numSquaresToEdge[row * 8 + col][13] = col >= 1 && row <= 5; // lefthighdown
            numSquaresToEdge[row * 8 + col][14] = col <= 6 && row <= 5; // righthighdown
            numSquaresToEdge[row * 8 + col][15] = col <= 5 && row <= 6; // rightmiddown

            // pawn specific (if it is possible that a pawn can move diagonal here)
            //     -> white pawn specific
            numSquaresToEdge[row * 8 + col][16] = (col >= 1 && row >= 1) ? (row * 8 + col - 9) : -1; // left capture
            numSquaresToEdge[row * 8 + col][17] = (col <= 6 && row >= 1) ? (row * 8 + col - 7) : -1; // right capture
            //     -> black pawn specific
            numSquaresToEdge[row * 8 + col][18] = (col >= 1 && row <= 6) ? (row * 8 + col + 7) : -1; // left capture
            numSquaresToEdge[row * 8 + col][19] = (col <= 6 && row <= 6) ? (row * 8 + col + 9) : -1; // right capture
        }
    }
}

LegalMovesContainer generatePseudoLegalMoves(Board *board)
{
    int legalMovesCapacity = 64;

    LegalMovesContainer legalMoves;
    legalMoves.amtOfMoves = 0;

    legalMoves.moves = malloc(legalMovesCapacity * sizeof(Move));
    if (legalMoves.moves == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    int toSquareIndex;
    int startDirection, endDirection;
    int pawnPushAmount, pawnDirection, pawnMoveIsCapture, capturedPieceIndex, onEnpassantSquare, enPassantPieceToCaptureIndex;
    int startPawnCaptureIndex, endPawnCaptureIndex;
    int isOkayToCastle;

    for (int squareIndex = 0; squareIndex < 64; squareIndex++)
    {
        // sliding pieces
        if (!(board->squares[squareIndex].type != NONE && board->squares[squareIndex].color == board->colorToPlay))
        {
            continue;
        }

        if (isSlidingPiece(board->squares[squareIndex]) || board->squares[squareIndex].type == KING)
        { // technically king isn't sliding but the code works here
            switch (board->squares[squareIndex].type)
            {
            case QUEEN:
                startDirection = 0;
                endDirection = 8;
                break;
            case BISHOP:
                startDirection = 4;
                endDirection = 8;
                break;
            case ROOK:
                startDirection = 0;
                endDirection = 4;
                break;
            case KING:
                startDirection = 0;
                endDirection = 8;
                break;
            }

            for (int directionOffsetIndex = startDirection; directionOffsetIndex < endDirection; directionOffsetIndex++)
            {
                for (int edgeStep = 1; edgeStep <= numSquaresToEdge[squareIndex][directionOffsetIndex]; edgeStep++)
                {
                    if (legalMoves.amtOfMoves >= legalMovesCapacity)
                    {
                        legalMovesCapacity *= 2;
                        legalMoves.moves = realloc(legalMoves.moves, legalMovesCapacity * sizeof(Move));

                        if (legalMoves.moves == NULL)
                        {
                            perror("malloc failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    toSquareIndex = board->squares[squareIndex].squareIndex + directionOffsets[directionOffsetIndex] * edgeStep;

                    if (toSquareIndex < 0 || toSquareIndex > 63)
                    {
                        continue;
                    }

                    if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color == board->colorToPlay)
                    {
                        // the move would capture your own piece, illegal
                        break;
                    }

                    if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color != board->colorToPlay)
                    {
                        // the move would capture the opponent's piece, stop after letting the move
                        // this is a capture move so register the capture square
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};
                        break;
                    } else {
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                    }
                    
                    if (board->squares[squareIndex].type == KING)
                    {
                        break;
                    }
                }
            }

            // extra requirements for king move to castle
            if (board->squares[squareIndex].type == KING) {
                // filter out the valid indexes based on current castling rights

                for (int i = (board->squares[squareIndex].color==WHITE_PIECE ? 0 : 2); i<(board->squares[squareIndex].color==WHITE_PIECE ? 2 : 4); i++) {
                    if (!board->castlingRights[i]) { continue; }

                    isOkayToCastle = 1;
                    for (int amtToAdd = 0; amtToAdd < 2; amtToAdd++)
                    {
                        
                        if (board->squares[kingIndexesToCastle[i*2+amtToAdd]].type != NONE) { 
                            isOkayToCastle = 0; 
                            break;
                        }
                    }
                    // take care of b1 and b8
                    if ((i == 1 || i == 3) && board->squares[kingIndexesToCastle[i*2+1]-1].type != NONE) { 
                        isOkayToCastle = 0; 
                    }
                    if (!isOkayToCastle) { continue; }

                    if (legalMoves.amtOfMoves >= legalMovesCapacity)
                    {
                        legalMovesCapacity *= 2;
                        legalMoves.moves = realloc(legalMoves.moves, legalMovesCapacity * sizeof(Move));

                        if (legalMoves.moves == NULL)
                        {
                            perror("malloc failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, kingIndexesToCastle[i*2+1], -1, (Square){NONE, NONE, -1}, 0};
                }

                

                
            }
            
        }
        else if (board->squares[squareIndex].type == KNIGHT)
        {
            for (int directionOffsetIndex = 8; directionOffsetIndex < 16; directionOffsetIndex++)
            {
                if (legalMoves.amtOfMoves >= legalMovesCapacity)
                {
                    legalMovesCapacity *= 2;
                    legalMoves.moves = realloc(legalMoves.moves, legalMovesCapacity * sizeof(Move));

                    if (legalMoves.moves == NULL)
                    {
                        perror("malloc failed");
                        exit(EXIT_FAILURE);
                    }
                }

                if (!numSquaresToEdge[squareIndex][directionOffsetIndex])
                {
                    continue;
                }

                toSquareIndex = board->squares[squareIndex].squareIndex + knightOffsets[directionOffsetIndex - 8];

                if (toSquareIndex < 0 || toSquareIndex > 63)
                {
                    continue;
                }

                if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color == board->colorToPlay)
                {
                    // the move would capture your own piece, illegal
                    continue;
                }

                if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color != board->colorToPlay)
                {
                    // this is a capture move so register the capture square
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};
                } else {
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                }
            }
        }
        else if (board->squares[squareIndex].type == PAWN)
        {
            // check for double push eligibility
            if (
                (board->squares[squareIndex].color == WHITE_PIECE && squareIndex >= 48 && squareIndex <= 55) ||
                (board->squares[squareIndex].color == BLACK_PIECE && squareIndex >= 8 && squareIndex <= 15))
            {
                pawnPushAmount = 2;
            }
            else
            {
                pawnPushAmount = 1;
            }

            pawnDirection = (board->squares[squareIndex].color == WHITE_PIECE) ? -8 : 8;

            for (int curPawnPushAmount = 1; curPawnPushAmount < pawnPushAmount + 1; curPawnPushAmount++)
            {
                if (legalMoves.amtOfMoves >= legalMovesCapacity)
                {
                    legalMovesCapacity *= 2;
                    legalMoves.moves = realloc(legalMoves.moves, legalMovesCapacity * sizeof(Move));

                    if (legalMoves.moves == NULL)
                    {
                        perror("malloc failed");
                        exit(EXIT_FAILURE);
                    }
                }

                toSquareIndex = board->squares[squareIndex].squareIndex + pawnDirection * curPawnPushAmount;

                if (toSquareIndex < 0 || toSquareIndex > 63)
                {
                    continue;
                }

                if (board->squares[toSquareIndex].type != NONE)
                {
                    // the pawn would walk into another piece, illegal
                    break;
                }

                legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
            }

            // pawn captures
            startPawnCaptureIndex = (board->squares[squareIndex].color == WHITE_PIECE) ? 16 : 18;
            endPawnCaptureIndex = (board->squares[squareIndex].color == WHITE_PIECE) ? 18 : 20;

            for (int pawnCaptureIndex = startPawnCaptureIndex; pawnCaptureIndex < endPawnCaptureIndex; pawnCaptureIndex++)
            {
                if (numSquaresToEdge[squareIndex][pawnCaptureIndex] != -1)
                {
                    if (legalMoves.amtOfMoves >= legalMovesCapacity)
                    {
                        legalMovesCapacity *= 2;
                        legalMoves.moves = realloc(legalMoves.moves, legalMovesCapacity * sizeof(Move));

                        if (legalMoves.moves == NULL)
                        {
                            perror("malloc failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    pawnMoveIsCapture = 0;
                    capturedPieceIndex = 0;
                    for (int enPassantIndex = 0; enPassantIndex < 64; enPassantIndex++)
                    {
                        // enPassantIndex is the the diagonal
                        if (!(enPassantIndex == numSquaresToEdge[squareIndex][pawnCaptureIndex]))
                        {
                            continue;
                        }

                        pawnDirection = (board->squares[squareIndex].color == WHITE_PIECE) ? 8 : -8;
                        enPassantPieceToCaptureIndex = enPassantIndex + pawnDirection;

                        if (enPassantPieceToCaptureIndex >= 0 || enPassantPieceToCaptureIndex <= 63)
                        {
                            if (board->squares[squareIndex].color == WHITE_PIECE)
                            {
                                onEnpassantSquare = (enPassantIndex >= 16 && enPassantIndex <= 23);
                            }
                            else
                            {
                                onEnpassantSquare = (enPassantIndex >= 40 && enPassantIndex <= 47);
                            }
                            onEnpassantSquare = (onEnpassantSquare && board->squares[enPassantPieceToCaptureIndex].type == PAWN &&  enPassantIndex == board->enPassantSquareIndex);
                        }
                        else
                        {
                            onEnpassantSquare = 0;
                        }

                        if (
                            // check if the piece can be captured by seeing if the square == the capture square
                            (
                                board->squares[enPassantIndex].type != NONE &&
                                board->squares[enPassantIndex].color != board->squares[squareIndex].color) ||
                            ( // en passant
                                board->squares[enPassantIndex].type == NONE &&
                                board->squares[enPassantIndex + pawnDirection].color != board->squares[squareIndex].color &&
                                onEnpassantSquare))
                        {
                            pawnMoveIsCapture = 1;
                            capturedPieceIndex = (!onEnpassantSquare) ? enPassantIndex : enPassantPieceToCaptureIndex;
                            break;
                        }
                    }
                    if (!pawnMoveIsCapture)
                    {
                        continue;
                    }

                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){board->squares[squareIndex].squareIndex, numSquaresToEdge[squareIndex][pawnCaptureIndex], -1, (Square){board->squares[capturedPieceIndex].type, board->squares[capturedPieceIndex].color, capturedPieceIndex}, onEnpassantSquare};
                }
            }
        }
    }
    return legalMoves;
}

LegalMovesContainer generateLegalMoves(Board *board) {
    LegalMovesContainer pseudoLegal = generatePseudoLegalMoves(board);

    LegalMovesContainer actualLegalMoves;
    actualLegalMoves.moves = malloc(pseudoLegal.amtOfMoves * sizeof(Move));
    actualLegalMoves.amtOfMoves = 0;
    if (actualLegalMoves.moves == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int isLegal;
    for (int pseudoLegalMoveIndex = 0; pseudoLegalMoveIndex < pseudoLegal.amtOfMoves; pseudoLegalMoveIndex++)
    {
        isLegal = 1;
        pushMove(board, pseudoLegal.moves[pseudoLegalMoveIndex]);
        LegalMovesContainer opponentResponseMoves = generatePseudoLegalMoves(board);
        for (int opponentResponseMoveIndex = 0; opponentResponseMoveIndex < opponentResponseMoves.amtOfMoves; opponentResponseMoveIndex++) {
            if (board->squares[opponentResponseMoves.moves[opponentResponseMoveIndex].toSquare].type == KING) {
                isLegal = 0;
            }
        }

        free(opponentResponseMoves.moves);

        if (isLegal) {
            actualLegalMoves.moves[actualLegalMoves.amtOfMoves++] = pseudoLegal.moves[pseudoLegalMoveIndex];
        } 

        
        popMove(board);
    }

    free(pseudoLegal.moves);
    
    return actualLegalMoves;
}

int moveGenerationTest(Board *board, int depth) {
    if (depth == 0) {
        return 1;
    }

    LegalMovesContainer legalMoves = generateLegalMoves(board);
    int numPos = 0;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        numPos += moveGenerationTest(board, depth-1);
        popMove(board);
    }

    free(legalMoves.moves);


    return numPos;
    
}

void initMoveGen()
{
    fillNumSquaresToEdge();
}
