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

Square generateAttackingSquares(Board *board, int colorOfFromPieceToAttack)
{
    AttackingSquareContainer *attackingSquares = (colorOfFromPieceToAttack == WHITE_PIECE) ? board->whiteAttackingSquares : board->blackAttackingSquares;
    int *amtAttackingSquares = (colorOfFromPieceToAttack == WHITE_PIECE) ? &board->whiteAttackingAmt : &board->blackAttackingAmt;
    *amtAttackingSquares = 0;

    Square *pieceSquares = (colorOfFromPieceToAttack == WHITE_PIECE) ? board->whitePieceSquares : board->blackPieceSquares;
    int amtOfPieces = (colorOfFromPieceToAttack == WHITE_PIECE) ? board->whitePieceAmt : board->blackPieceAmt;

    int startDirection, endDirection, toSquareIndex;
    int canAddAttacker;
    int pawnPushAmount, pawnDirection, pawnMoveIsCapture, capturedPieceIndex, onEnpassantSquare, enPassantPieceToCaptureIndex;
    int pawnCaptureIndexesToTry[2];
    int pieceCapturedByPawnIndex;
    int startPawnCaptureIndex, endPawnCaptureIndex;

    for (int squareIndex = 0; squareIndex < amtOfPieces; squareIndex++)
    {
        // sliding pieces
        if (isSlidingPiece(pieceSquares[squareIndex]) || pieceSquares[squareIndex].type == KING)
        { // technically king isn't sliding but the code works here
            switch (pieceSquares[squareIndex].type)
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
                for (int edgeStep = 1; edgeStep <= numSquaresToEdge[pieceSquares[squareIndex].squareIndex][directionOffsetIndex]; edgeStep++)
                {
                    toSquareIndex = pieceSquares[squareIndex].squareIndex + directionOffsets[directionOffsetIndex] * edgeStep;

                    if (toSquareIndex < 0 || toSquareIndex > 63)
                    {
                        continue;
                    }

                    canAddAttacker = 1;
                    for (int verifyExistenceIdx = 0; verifyExistenceIdx < *amtAttackingSquares; verifyExistenceIdx++)
                    {
                        if (attackingSquares[verifyExistenceIdx].attackingSquare.squareIndex == toSquareIndex)
                        {
                            if (attackingSquares[verifyExistenceIdx].attackGivenFromSquareIndex == pieceSquares[squareIndex].squareIndex)
                            {
                                // cannot add duplicates, but can add other squares attacking the same square
                                canAddAttacker = 0;
            
                                break;
                            }
                        }
                    }

                    

                    if (board->squares[toSquareIndex].type != NONE || pieceSquares[squareIndex].type == KING)
                    {
                        // the move would capture another piece, stop here
                        if (canAddAttacker && board->squares[toSquareIndex].type == KING && board->squares[toSquareIndex].color != colorOfFromPieceToAttack)
                        {
                            attackingSquares[(*amtAttackingSquares)++] = (AttackingSquareContainer){
                                (Square){
                                    board->squares[toSquareIndex].type,
                                    board->squares[toSquareIndex].color,
                                    toSquareIndex},
                                pieceSquares[squareIndex].squareIndex};
                        }
                        break;
                    }
                }
            }
        }
        else if (pieceSquares[squareIndex].type == KNIGHT)
        {
            for (int directionOffsetIndex = 8; directionOffsetIndex < 16; directionOffsetIndex++)
            {
                if (!numSquaresToEdge[pieceSquares[squareIndex].squareIndex][directionOffsetIndex])
                {
                    continue;
                }

                toSquareIndex = pieceSquares[squareIndex].squareIndex + knightOffsets[directionOffsetIndex - 8];

                if (toSquareIndex < 0 || toSquareIndex > 63)
                {
                    continue;
                }

                canAddAttacker = 1;
                for (int verifyExistenceIdx = 0; verifyExistenceIdx < *amtAttackingSquares; verifyExistenceIdx++)
                {
                    if (attackingSquares[verifyExistenceIdx].attackingSquare.squareIndex == toSquareIndex)
                    {
                        if (attackingSquares[verifyExistenceIdx].attackGivenFromSquareIndex == pieceSquares[squareIndex].squareIndex)
                        {
                            // cannot add duplicates, but can add other squares attacking the same square
                            canAddAttacker = 0;
                            break;
                        }
                    }
                }

                if (canAddAttacker && board->squares[toSquareIndex].type == KING && board->squares[toSquareIndex].color != colorOfFromPieceToAttack)
                {
                    attackingSquares[(*amtAttackingSquares)++] = (AttackingSquareContainer){
                        (Square){
                            board->squares[toSquareIndex].type,
                            board->squares[toSquareIndex].color,
                            toSquareIndex},
                        pieceSquares[squareIndex].squareIndex};
                }
            }
        }
        else if (pieceSquares[squareIndex].type == PAWN)
        {
            // pawn captures
            startPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 16 : 18;
            endPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 18 : 20;

            pawnDirection = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 8 : -8;
            if (pieceSquares[squareIndex].color == WHITE_PIECE)
            {
                pawnCaptureIndexesToTry[0] = pieceSquares[squareIndex].squareIndex - 9;
                pawnCaptureIndexesToTry[1] = pieceSquares[squareIndex].squareIndex - 7;
            }
            else
            {
                pawnCaptureIndexesToTry[0] = pieceSquares[squareIndex].squareIndex + 7;
                pawnCaptureIndexesToTry[1] = pieceSquares[squareIndex].squareIndex + 9;
            }

            int pawnCaptureIndexesToTryIndex = -1; // * because we do ++ right after (don't change)
            for (int pawnCaptureIndex = startPawnCaptureIndex; pawnCaptureIndex < endPawnCaptureIndex; pawnCaptureIndex++)
            {
                pawnCaptureIndexesToTryIndex++;
                if (numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex] != -1)
                {
                    pawnMoveIsCapture = 0;
                    capturedPieceIndex = 0;

                    // pieceCapturedByPawnIndex is the the diagonal
                    pieceCapturedByPawnIndex = pawnCaptureIndexesToTry[pawnCaptureIndexesToTryIndex];

                    enPassantPieceToCaptureIndex = pieceCapturedByPawnIndex + pawnDirection;

                    if (enPassantPieceToCaptureIndex >= 0 || enPassantPieceToCaptureIndex <= 63)
                    {
                        if (pieceSquares[squareIndex].color == WHITE_PIECE)
                        {
                            onEnpassantSquare = (pieceCapturedByPawnIndex >= 16 && pieceCapturedByPawnIndex <= 23);
                        }
                        else
                        {
                            onEnpassantSquare = (pieceCapturedByPawnIndex >= 40 && pieceCapturedByPawnIndex <= 47);
                        }
                        onEnpassantSquare = (onEnpassantSquare && board->squares[enPassantPieceToCaptureIndex].type == PAWN && pieceCapturedByPawnIndex == board->enPassantSquareIndex);
                    }
                    else
                    {
                        onEnpassantSquare = 0;
                    }

                    if (
                        // check if the piece can be captured by seeing if the square == the capture square
                        (
                            board->squares[pieceCapturedByPawnIndex].type != NONE &&
                            board->squares[pieceCapturedByPawnIndex].color != pieceSquares[squareIndex].color) ||
                        ( // en passant
                            board->squares[pieceCapturedByPawnIndex].type == NONE &&
                            board->squares[pieceCapturedByPawnIndex + pawnDirection].color != pieceSquares[squareIndex].color &&
                            onEnpassantSquare))
                    {
                        pawnMoveIsCapture = 1;
                        capturedPieceIndex = (!onEnpassantSquare) ? pieceCapturedByPawnIndex : enPassantPieceToCaptureIndex;
                    }
                    else
                    {
                        capturedPieceIndex = pieceCapturedByPawnIndex;
                    }

                    canAddAttacker = 1;
                    for (int verifyExistenceIdx = 0; verifyExistenceIdx < *amtAttackingSquares; verifyExistenceIdx++)
                    {
                        if (attackingSquares[verifyExistenceIdx].attackingSquare.squareIndex == capturedPieceIndex)
                        {
                            if (attackingSquares[verifyExistenceIdx].attackGivenFromSquareIndex == pieceSquares[squareIndex].squareIndex)
                            {
                                // cannot add duplicates, but can add other squares attacking the same square
                                canAddAttacker = 0;
            
                                break;
                            }
                        }
                    }

                    if (canAddAttacker && board->squares[capturedPieceIndex].type == KING && board->squares[capturedPieceIndex].color != colorOfFromPieceToAttack)
                    {
                        attackingSquares[(*amtAttackingSquares)++] = (AttackingSquareContainer){
                            (Square){
                                board->squares[capturedPieceIndex].type,
                                board->squares[capturedPieceIndex].color,
                                capturedPieceIndex},
                            pieceSquares[squareIndex].squareIndex};
                    }
                }
            }
        }
    }
}

LegalMovesContainer generatePseudoLegalMoves(Board *board)
{
    int legalMovesCapacity = 35;

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

    int pawnCaptureIndexesToTry[2];
    int pieceCapturedByPawnIndex;
    int pawnCanPromote;

    int startPawnCaptureIndex, endPawnCaptureIndex;
    int isOkayToCastle;

    Square *pieceSquares = (board->colorToPlay == WHITE_PIECE) ? board->whitePieceSquares : board->blackPieceSquares;
    int amtOfPieces = (board->colorToPlay == WHITE_PIECE) ? board->whitePieceAmt : board->blackPieceAmt;

    for (int squareIndex = 0; squareIndex < amtOfPieces; squareIndex++)
    {
        // sliding pieces
        if (isSlidingPiece(pieceSquares[squareIndex]) || pieceSquares[squareIndex].type == KING)
        { // technically king isn't sliding but the code works here
            switch (pieceSquares[squareIndex].type)
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
                for (int edgeStep = 1; edgeStep <= numSquaresToEdge[pieceSquares[squareIndex].squareIndex][directionOffsetIndex]; edgeStep++)
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

                    toSquareIndex = pieceSquares[squareIndex].squareIndex + directionOffsets[directionOffsetIndex] * edgeStep;

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
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};

                        break;
                    }
                    else
                    {
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                    }

                    if (pieceSquares[squareIndex].type == KING)
                    {
                        break;
                    }
                }
            }

            // extra requirements for king move to castle
            if (pieceSquares[squareIndex].type == KING)
            {
                // filter out the valid indexes based on current castling rights
                for (int i = (pieceSquares[squareIndex].color == WHITE_PIECE ? 0 : 2); i < (pieceSquares[squareIndex].color == WHITE_PIECE ? 2 : 4); i++)
                {
                    if (!board->castlingRights[i])
                    {
                        continue;
                    }

                    isOkayToCastle = 1;
                    for (int amtToAdd = 0; amtToAdd < 2; amtToAdd++)
                    {

                        if (board->squares[kingIndexesToCastle[i * 2 + amtToAdd]].type != NONE)
                        {
                            isOkayToCastle = 0;
                            break;
                        }

                        board->squares[kingIndexesToCastle[i * 2 + amtToAdd]] = (Square){KING, board->colorToPlay, kingIndexesToCastle[i * 2 + amtToAdd]};
                        
                        generateAttackingSquares(board, (board->colorToPlay == BLACK_PIECE) ? WHITE_PIECE : BLACK_PIECE);
                        
                        board->squares[kingIndexesToCastle[i * 2 + amtToAdd]] = (Square){NONE, NONE, kingIndexesToCastle[i * 2 + amtToAdd]};

                        if (((board->colorToPlay == WHITE_PIECE) ? board->blackAttackingAmt : board->whiteAttackingAmt) != 0)
                        {   
                            isOkayToCastle = 0;
                            break;
                        }

                    }
                    // take care of b1 and b8
                    if ((i == 1 || i == 3) && board->squares[kingIndexesToCastle[i * 2 + 1] - 1].type != NONE)
                    {
                        isOkayToCastle = 0;
                    }
                    if (!isOkayToCastle)
                    {
                        continue;
                    }

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
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, kingIndexesToCastle[i * 2 + 1], -1, (Square){NONE, NONE, -1}, 0};
                }
            }
        }
        else if (pieceSquares[squareIndex].type == KNIGHT)
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

                if (!numSquaresToEdge[pieceSquares[squareIndex].squareIndex][directionOffsetIndex])
                {
                    continue;
                }

                toSquareIndex = pieceSquares[squareIndex].squareIndex + knightOffsets[directionOffsetIndex - 8];

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
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};
                }
                else
                {
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                }
            }
        }
        else if (pieceSquares[squareIndex].type == PAWN)
        {
            // check for double push eligibility
            if (
                (pieceSquares[squareIndex].color == WHITE_PIECE && pieceSquares[squareIndex].squareIndex >= 48 && pieceSquares[squareIndex].squareIndex <= 55) ||
                (pieceSquares[squareIndex].color == BLACK_PIECE && pieceSquares[squareIndex].squareIndex >= 8 && pieceSquares[squareIndex].squareIndex <= 15))
            {
                pawnPushAmount = 2;
            }
            else
            {
                pawnPushAmount = 1;
            }

            pawnDirection = (pieceSquares[squareIndex].color == WHITE_PIECE) ? -8 : 8;

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

                toSquareIndex = pieceSquares[squareIndex].squareIndex + pawnDirection * curPawnPushAmount;

                if (toSquareIndex < 0 || toSquareIndex > 63)
                {
                    continue;
                }

                if (board->squares[toSquareIndex].type != NONE)
                {
                    // the pawn would walk into another piece, illegal
                    break;
                }
                
                pawnCanPromote = 0;
                if (board->colorToPlay == WHITE_PIECE && toSquareIndex / 8 == 0) {
                    // the white pawn has gotten to be able to promote
                    pawnCanPromote = 1;
                } else if (board->colorToPlay == BLACK_PIECE && toSquareIndex / 8 == 7) {
                    // the black pawn has gotten to be able to promote
                    pawnCanPromote = 1;                    
                }
                
                if (!pawnCanPromote) {
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                } else {
                    for (int promotionType = KNIGHT; promotionType < KING; promotionType++)
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
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, toSquareIndex, promotionType, (Square){NONE, NONE, -1}, 0};   
                    }
                }
            }

            // pawn captures
            startPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 16 : 18;
            endPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 18 : 20;

            pawnDirection = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 8 : -8;
            if (pieceSquares[squareIndex].color == WHITE_PIECE)
            {
                pawnCaptureIndexesToTry[0] = pieceSquares[squareIndex].squareIndex - 9;
                pawnCaptureIndexesToTry[1] = pieceSquares[squareIndex].squareIndex - 7;
            }
            else
            {
                pawnCaptureIndexesToTry[0] = pieceSquares[squareIndex].squareIndex + 7;
                pawnCaptureIndexesToTry[1] = pieceSquares[squareIndex].squareIndex + 9;
            }

            int pawnCaptureIndexesToTryIndex = -1; // * because we do ++ right after (don't change)
            for (int pawnCaptureIndex = startPawnCaptureIndex; pawnCaptureIndex < endPawnCaptureIndex; pawnCaptureIndex++)
            {
                pawnCaptureIndexesToTryIndex++;
                if (numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex] != -1)
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

                    // pieceCapturedByPawnIndex is the the diagonal
                    pieceCapturedByPawnIndex = pawnCaptureIndexesToTry[pawnCaptureIndexesToTryIndex];

                    enPassantPieceToCaptureIndex = pieceCapturedByPawnIndex + pawnDirection;

                    if (enPassantPieceToCaptureIndex >= 0 || enPassantPieceToCaptureIndex <= 63)
                    {
                        if (pieceSquares[squareIndex].color == WHITE_PIECE)
                        {
                            onEnpassantSquare = (pieceCapturedByPawnIndex >= 16 && pieceCapturedByPawnIndex <= 23);
                        }
                        else
                        {
                            onEnpassantSquare = (pieceCapturedByPawnIndex >= 40 && pieceCapturedByPawnIndex <= 47);
                        }
                        onEnpassantSquare = (onEnpassantSquare && board->squares[enPassantPieceToCaptureIndex].type == PAWN && pieceCapturedByPawnIndex == board->enPassantSquareIndex);
                    }
                    else
                    {
                        onEnpassantSquare = 0;
                    }

                    if (
                        // check if the piece can be captured by seeing if the square == the capture square
                        (
                            board->squares[pieceCapturedByPawnIndex].type != NONE &&
                            board->squares[pieceCapturedByPawnIndex].color != pieceSquares[squareIndex].color) ||
                        ( // en passant
                            board->squares[pieceCapturedByPawnIndex].type == NONE &&
                            board->squares[pieceCapturedByPawnIndex + pawnDirection].color != pieceSquares[squareIndex].color &&
                            onEnpassantSquare))
                    {
                        pawnMoveIsCapture = 1;
                        capturedPieceIndex = (!onEnpassantSquare) ? pieceCapturedByPawnIndex : enPassantPieceToCaptureIndex;
                    }
                    if (!pawnMoveIsCapture)
                    {
                        continue;
                    }

                    pawnCanPromote = 0;
                    if (board->colorToPlay == WHITE_PIECE && numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex] / 8 == 0) {
                        // the white pawn has gotten to be able to promote
                        pawnCanPromote = 1;
                    } else if (board->colorToPlay == BLACK_PIECE && numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex] / 8 == 7) {
                        // the black pawn has gotten to be able to promote
                        pawnCanPromote = 1;                    
                    }
                    
                    if (!pawnCanPromote) {
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex], -1, (Square){board->squares[capturedPieceIndex].type, board->squares[capturedPieceIndex].color, capturedPieceIndex}, onEnpassantSquare};
                    } else {
                        for (int promotionType = KNIGHT; promotionType < KING; promotionType++)
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
                            legalMoves.moves[legalMoves.amtOfMoves++] = (Move){pieceSquares[squareIndex].squareIndex, numSquaresToEdge[pieceSquares[squareIndex].squareIndex][pawnCaptureIndex], promotionType, (Square){board->squares[capturedPieceIndex].type, board->squares[capturedPieceIndex].color, capturedPieceIndex}, onEnpassantSquare};
                        }
                    }
                }
            }
        }
    }
    return legalMoves;
}



LegalMovesContainer generateLegalMoves(Board *board)
{
    if (board->gameState != 0) {
        // if checkmate or stalemate, stop wasting time trying to generate legal moves
        LegalMovesContainer actualLegalMoves;
        return actualLegalMoves;
    }
    LegalMovesContainer pseudoLegal = generatePseudoLegalMoves(board);
    // return pseudoLegal;

    LegalMovesContainer actualLegalMoves;
    actualLegalMoves.moves = malloc(pseudoLegal.amtOfMoves * sizeof(Move));
    actualLegalMoves.amtOfMoves = 0;
    if (actualLegalMoves.moves == NULL)
    {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }


    int originalColorOfFromPieceToAttack = (board->colorToPlay == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;

    for (int pseudoLegalMoveIndex = 0; pseudoLegalMoveIndex < pseudoLegal.amtOfMoves; pseudoLegalMoveIndex++)
    {   
        pushMove(board, pseudoLegal.moves[pseudoLegalMoveIndex]);

        generateAttackingSquares(board, originalColorOfFromPieceToAttack);

        if (((originalColorOfFromPieceToAttack == BLACK_PIECE) ? board->blackAttackingAmt : board->whiteAttackingAmt) == 0)
        {
            actualLegalMoves.moves[actualLegalMoves.amtOfMoves++] = pseudoLegal.moves[pseudoLegalMoveIndex];
        }

        popMove(board);
    }

    free(pseudoLegal.moves);

    if (actualLegalMoves.amtOfMoves == 0) {
        generateAttackingSquares(board, originalColorOfFromPieceToAttack);
        if (((originalColorOfFromPieceToAttack == BLACK_PIECE) ? board->blackAttackingAmt : board->whiteAttackingAmt) == 0) {
            // no checks, is stalemate
            board->gameState = STALEMATE;
        } else {
            // there are check, is checkmate
            board->gameState = CHECKMATE;
        }
    }

    return actualLegalMoves;
}

int moveGenerationTest(Board *board, int depth)
{
    if (depth == 0)
    {
        return 1;
    }

    LegalMovesContainer legalMoves = generateLegalMoves(board);
    int numPos = 0;
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        pushMove(board, legalMoves.moves[i]);
        numPos += moveGenerationTest(board, depth - 1);
        popMove(board);
    }

    free(legalMoves.moves);

    return numPos;
}

void initMoveGen()
{
    fillNumSquaresToEdge();
}
