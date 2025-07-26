#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

    int kingPieceSquaresIndexOfColorToPlay;
    int kingBoardSquareOfColorToPlay;
    for (int i = 0; i < amtOfPieces; i++) {
        if (pieceSquares[i].type == KING) {
            kingPieceSquaresIndexOfColorToPlay = i;
            kingBoardSquareOfColorToPlay = pieceSquares[i].squareIndex;
            break;
        }
    }

    int actualPieceSquareIndex;
    for (int squareIndex = 0; squareIndex < amtOfPieces; squareIndex++)
    {
        actualPieceSquareIndex = pieceSquares[squareIndex].squareIndex;
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
                for (int edgeStep = 1; edgeStep <= numSquaresToEdge[actualPieceSquareIndex][directionOffsetIndex]; edgeStep++)
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

                    toSquareIndex = actualPieceSquareIndex + directionOffsets[directionOffsetIndex] * edgeStep;

                    if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color == board->colorToPlay)
                    {
                        // the move would capture your own piece, illegal
                        break;
                    }

                    if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color != board->colorToPlay)
                    {
                        // the move would capture the opponent's piece, stop after letting the move
                        // this is a capture move so register the capture square
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};

                        break;
                    }
                    else
                    {
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
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
                    if (kingIsAttacked(board, board->colorToPlay)) {
                        break;
                    }


                    isOkayToCastle = 1;
                    for (int amtToAdd = 0; amtToAdd < 2; amtToAdd++)
                    {

                        if (board->squares[kingIndexesToCastle[i * 2 + amtToAdd]].type != NONE)
                        {
                            isOkayToCastle = 0;
                            break;
                        }

                        // remove the actual king first, then reset it
                        // pieceSquares[kingIndexOfColorToPlay] = (Square){NONE, NONE, -1};
                        
                        
                        board->squares[kingBoardSquareOfColorToPlay] = (Square){NONE, NONE, kingBoardSquareOfColorToPlay};
                        pieceSquares[kingPieceSquaresIndexOfColorToPlay] = (Square){KING, board->colorToPlay, kingIndexesToCastle[i * 2 + amtToAdd]};
                        
                        board->squares[kingIndexesToCastle[i * 2 + amtToAdd]] = (Square){KING, board->colorToPlay, kingIndexesToCastle[i * 2 + amtToAdd]};
                        isOkayToCastle = !kingIsAttacked(board, board->colorToPlay);                       
                        board->squares[kingIndexesToCastle[i * 2 + amtToAdd]] = (Square){NONE, NONE, kingIndexesToCastle[i * 2 + amtToAdd]};
                        
                        pieceSquares[kingPieceSquaresIndexOfColorToPlay] = (Square){KING, board->colorToPlay, kingBoardSquareOfColorToPlay};
                        board->squares[kingBoardSquareOfColorToPlay] = (Square){KING, board->colorToPlay, kingBoardSquareOfColorToPlay};
                        if (!isOkayToCastle)
                        {   
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
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, kingIndexesToCastle[i * 2 + 1], -1, (Square){NONE, NONE, -1}, 0};
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

                if (!numSquaresToEdge[actualPieceSquareIndex][directionOffsetIndex])
                {
                    continue;
                }

                toSquareIndex = actualPieceSquareIndex + knightOffsets[directionOffsetIndex - 8];

                if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color == board->colorToPlay)
                {
                    // the move would capture your own piece, illegal
                    continue;
                }

                if (board->squares[toSquareIndex].type != NONE && board->squares[toSquareIndex].color != board->colorToPlay)
                {
                    // this is a capture move so register the capture square
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, -1, (Square){board->squares[toSquareIndex].type, board->squares[toSquareIndex].color, toSquareIndex}, 0};
                }
                else
                {
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
                }
            }
        }
        else if (pieceSquares[squareIndex].type == PAWN)
        {
            // check for double push eligibility
            if (
                (pieceSquares[squareIndex].color == WHITE_PIECE && actualPieceSquareIndex >= 48 && actualPieceSquareIndex <= 55) ||
                (pieceSquares[squareIndex].color == BLACK_PIECE && actualPieceSquareIndex >= 8 && actualPieceSquareIndex <= 15))
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

                toSquareIndex = actualPieceSquareIndex + pawnDirection * curPawnPushAmount;

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
                    legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, -1, (Square){NONE, NONE, -1}, 0};
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
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, toSquareIndex, promotionType, (Square){NONE, NONE, -1}, 0};   
                    }
                }
            }

            // pawn captures
            startPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 16 : 18;
            endPawnCaptureIndex = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 18 : 20;

            pawnDirection = (pieceSquares[squareIndex].color == WHITE_PIECE) ? 8 : -8;
            if (pieceSquares[squareIndex].color == WHITE_PIECE)
            {
                pawnCaptureIndexesToTry[0] = actualPieceSquareIndex - 9;
                pawnCaptureIndexesToTry[1] = actualPieceSquareIndex - 7;
            }
            else
            {
                pawnCaptureIndexesToTry[0] = actualPieceSquareIndex + 7;
                pawnCaptureIndexesToTry[1] = actualPieceSquareIndex + 9;
            }

            int pawnCaptureIndexesToTryIndex = -1; // * because we do ++ right after (don't change)
            for (int pawnCaptureIndex = startPawnCaptureIndex; pawnCaptureIndex < endPawnCaptureIndex; pawnCaptureIndex++)
            {
                pawnCaptureIndexesToTryIndex++;
                if (numSquaresToEdge[actualPieceSquareIndex][pawnCaptureIndex] != -1)
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

                    if (pieceSquares[squareIndex].color == WHITE_PIECE)
                    {
                        onEnpassantSquare = (pieceCapturedByPawnIndex >= 16 && pieceCapturedByPawnIndex <= 23);
                    }
                    else
                    {
                        onEnpassantSquare = (pieceCapturedByPawnIndex >= 40 && pieceCapturedByPawnIndex <= 47);
                    }
                    onEnpassantSquare = (onEnpassantSquare && board->squares[enPassantPieceToCaptureIndex].type == PAWN && pieceCapturedByPawnIndex == board->enPassantSquareIndex);


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
                    if (board->colorToPlay == WHITE_PIECE && numSquaresToEdge[actualPieceSquareIndex][pawnCaptureIndex] / 8 == 0) {
                        // the white pawn has gotten to be able to promote
                        pawnCanPromote = 1;
                    } else if (board->colorToPlay == BLACK_PIECE && numSquaresToEdge[actualPieceSquareIndex][pawnCaptureIndex] / 8 == 7) {
                        // the black pawn has gotten to be able to promote
                        pawnCanPromote = 1;                    
                    }
                    
                    if (!pawnCanPromote) {
                        legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, numSquaresToEdge[actualPieceSquareIndex][pawnCaptureIndex], -1, (Square){board->squares[capturedPieceIndex].type, board->squares[capturedPieceIndex].color, capturedPieceIndex}, onEnpassantSquare};
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
                            legalMoves.moves[legalMoves.amtOfMoves++] = (Move){actualPieceSquareIndex, numSquaresToEdge[actualPieceSquareIndex][pawnCaptureIndex], promotionType, (Square){board->squares[capturedPieceIndex].type, board->squares[capturedPieceIndex].color, capturedPieceIndex}, onEnpassantSquare};
                        }
                    }
                }
            }
        }
    }
    return legalMoves;
}

int kingIsAttacked(Board *board, int colorToCheck) {
    int kingSquare = -1;
    int enemyColor = (colorToCheck == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;

    // find the king's square
    Square *pieceSquares = (colorToCheck == WHITE_PIECE) ? board->whitePieceSquares : board->blackPieceSquares;
    int amtOfPieces = (colorToCheck == WHITE_PIECE) ? board->whitePieceAmt : board->blackPieceAmt;

    for (int i = 0; i < amtOfPieces; i++) {
        if (pieceSquares[i].type == KING) {
            kingSquare = pieceSquares[i].squareIndex;
            break;
        }
    }

    if (kingSquare == -1) {
        fprintf(stderr, "kingIsAttacked: could not find king for color %d\n", colorToCheck);
        exit(1);
    }
    // ? the type checks are to see if there does exist a piece of that type 
    // ? in that direction that is attacking the king

    // Step 2: knight attacks
    for (int i = 0; i < 8; i++) {
        int to = kingSquare + knightOffsets[i];
        if (!numSquaresToEdge[kingSquare][i + 8]) continue;
        Square target = board->squares[to];
        if (target.type == KNIGHT && target.color == enemyColor) return 1;
    }

    // Step 3: pawn attacks
    int pawnDirs[2] = {
        (colorToCheck == WHITE_PIECE) ? 16 : 18,
        (colorToCheck == WHITE_PIECE) ? 17 : 19
    };

    int pawnCaptureIndexesToTry[2];

    if (colorToCheck == WHITE_PIECE)
    {
        pawnCaptureIndexesToTry[0] = kingSquare - 9;
        pawnCaptureIndexesToTry[1] = kingSquare - 7;
    }
    else
    {
        pawnCaptureIndexesToTry[0] = kingSquare + 7;
        pawnCaptureIndexesToTry[1] = kingSquare + 9;
    }
    
    for (int i = 0; i < 2; i++) {
        if (numSquaresToEdge[kingSquare][pawnDirs[i]] == -1) continue;
        int to = pawnCaptureIndexesToTry[i];
        Square target = board->squares[to];
        if (target.type == PAWN && target.color == enemyColor) return 1;
    }

    // Step 4: sliding attacks (rook, bishop, queen)
    for (int dir = 0; dir < 8; dir++) {
        for (int dist = 1; dist <= numSquaresToEdge[kingSquare][dir]; dist++) {
            int to = kingSquare + directionOffsets[dir] * dist;

            Square target = board->squares[to];

            if (target.type == NONE) continue;
            if (target.color == colorToCheck) break;

            if ((dir < 4 && (target.type == ROOK || target.type == QUEEN)) ||
                (dir >= 4 && (target.type == BISHOP || target.type == QUEEN)))
                return 1;

            break; // blocked by enemy piece that's not attacking
        }
    }

    // Step 5: adjacent enemy king
    for (int dir = 0; dir < 8; dir++) {
        if (!numSquaresToEdge[kingSquare][dir]) continue;
        int to = kingSquare + directionOffsets[dir];
        Square target = board->squares[to];
        if (target.type == KING && target.color == enemyColor) return 1;
    }

    return 0;
}

LegalMovesContainer generateLegalMoves(Board *board)
{
    if (board->gameState > CHECK) {
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
    int originalColorToPlay = board->colorToPlay;

    for (int pseudoLegalMoveIndex = 0; pseudoLegalMoveIndex < pseudoLegal.amtOfMoves; pseudoLegalMoveIndex++)
    {   
        pushMove(board, pseudoLegal.moves[pseudoLegalMoveIndex]);

        
        if (!kingIsAttacked(board, originalColorToPlay))
        {
            actualLegalMoves.moves[actualLegalMoves.amtOfMoves++] = pseudoLegal.moves[pseudoLegalMoveIndex];
        }

        popMove(board);
    }

    free(pseudoLegal.moves);

    if (actualLegalMoves.amtOfMoves == 0) {
        if (!kingIsAttacked(board, originalColorToPlay)) {
            // no checks, is stalemate
            board->gameState = STALEMATE;
        } else {
            // there are check, is checkmate
            board->gameState = CHECKMATE;
        }
    } else {
        if (kingIsAttacked(board, originalColorToPlay)) {
            board->gameState = CHECK;
        } else {
            board->gameState = NONE;
        }
    }
    
    return actualLegalMoves;
}

// void indexToNotation(int squareIndex, char *notation) {
//     int file = squareIndex % 8;  // 0 = 'a', ..., 7 = 'h'
//     int rank = 7 - (squareIndex / 8);  // 0 = '8', ..., 7 = '1'

//     notation[0] = 'a' + file;
//     notation[1] = '1' + rank;
//     notation[2] = '\0';
// }

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
// int moveGenerationTest(Board *board, int depth)
// {
//     if (depth == 0)
//     {
//         LegalMovesContainer legalMoves = generateLegalMoves(board);
//         return 1;
//     }


//     LegalMovesContainer legalMoves = generateLegalMoves(board);
//     int numPos = 0;
//     int amt;
//     for (int i = 0; i < legalMoves.amtOfMoves; i++)
//     {
//         pushMove(board, legalMoves.moves[i]);

//         amt = moveGenerationTest(board, depth - 1);
//         numPos += amt;
//         if (depth == 6) {
//             char notation[3];
//             int index = legalMoves.moves[i].fromSquare; 
//             indexToNotation(index, notation);
//             char notation2[3];
//             int index2 = legalMoves.moves[i].toSquare; 
//             indexToNotation(index2, notation2);
//             printf("\n%s%s -> %d", notation, notation2, amt);
            
//         }
//         popMove(board);
//     }

//     free(legalMoves.moves);

//     return numPos;
// }

void initMoveGen()
{
    fillNumSquaresToEdge();
}
