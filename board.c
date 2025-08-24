#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "board.h"
#include "movegen.h"
#include "zobrist.h"

void convertCharToPieceType(char pieceChar, int *pieceType, int *pieceColor)
{
    /* use like:
    char pieceChar = 'K';
    int pieceType, pieceColor;
    convertCharToPieceType(pieceChar, &pieceType, &pieceColor);
    */

    switch (tolower(pieceChar))
    {
    case 'p':
        *pieceType = PAWN;
        break;
    case 'n':
        *pieceType = KNIGHT;
        break;
    case 'b':
        *pieceType = BISHOP;
        break;
    case 'r':
        *pieceType = ROOK;
        break;
    case 'q':
        *pieceType = QUEEN;
        break;
    case 'k':
        *pieceType = KING;
        break;
    case ' ':
        *pieceType = NONE;
        *pieceColor = NONE;
        return;
    }

    if (isupper(pieceChar))
    {
        *pieceColor = WHITE_PIECE;
    }
    else
    {
        *pieceColor = BLACK_PIECE;
    }
}

void convertPieceTypeToChar(int pieceType, int pieceColor, char *pieceChar)
{
    /* use like:
    int pieceType = ROOK;
    int pieceColor = WHITE_PIECE;
    char pieceChar;
    convertCharToPieceType(pieceType, pieceColor, &pieceChar);
    */

    switch (pieceType)
    {
    case PAWN:
        *pieceChar = 'p';
        break;
    case KNIGHT:
        *pieceChar = 'n';
        break;
    case BISHOP:
        *pieceChar = 'b';
        break;
    case ROOK:
        *pieceChar = 'r';
        break;
    case QUEEN:
        *pieceChar = 'q';
        break;
    case KING:
        *pieceChar = 'k';
        break;
    case NONE:
        *pieceChar = ' ';
        break;
    }
    if (pieceColor == WHITE_PIECE)
    {
        *pieceChar = toupper(*pieceChar);
    }
}

void printBoard(Board *board)
{
    printf("-----------------\n");
    for (int i = 0; i < 64; i++)
    {
        if (i % 8 == 0 && i != 0)
        {
            printf("\n");
            printf("-----------------\n");
        }
        char pieceChar;
        convertPieceTypeToChar(board->squares[i].type, board->squares[i].color, &pieceChar);
        printf("|%c", pieceChar);
        if ((i + 1) % 8 == 0)
        {
            printf("|");
        }
    }
    printf("\n-----------------\n");
}

void printMove(Move move)
{
    printf("pushMove(&board, (Move){%d, %d, %d, (Square){%d, %d, %d}, %d});\n", move.fromSquare, move.toSquare, move.promotionType, move.captureSquare.type, move.captureSquare.color, move.captureSquare.squareIndex, move.isEnpassant);
}

void pushMove(Board *board, Move move)
{
    if (board->moves.size >= board->moves.capacity)
    {
        board->moves.capacity *= 2;
        board->moves.stack = realloc(board->moves.stack, board->moves.capacity * sizeof(UndoMove));
        if (board->moves.stack == NULL)
        {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
    }

    uint64_t stateZobristHash = generateZobristHash(board);

    board->moves.stack[board->moves.size].oldEnPassantSquareIndex = board->enPassantSquareIndex;
    memcpy(board->moves.stack[board->moves.size].oldCastlingRights, board->castlingRights, 4 * sizeof(int));
    board->moves.stack[board->moves.size].oldHalfMoveClock = board->halfmoveClock;

    board->moves.stack[board->moves.size].oldWhitePieceAmt = board->whitePieceAmt;
    memcpy(board->moves.stack[board->moves.size].oldWhitePieceSquares, board->whitePieceSquares, 16 * sizeof(Square));

    board->moves.stack[board->moves.size].oldBlackPieceAmt = board->blackPieceAmt;
    memcpy(board->moves.stack[board->moves.size].oldBlackPieceSquares, board->blackPieceSquares, 16 * sizeof(Square));
    board->moves.stack[board->moves.size].oldGameState = board->gameState;

    board->moves.stack[board->moves.size].oldZobristHash = stateZobristHash;

    // change the board turn
    board->colorToPlay = (board->colorToPlay == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;

    if (board->colorToPlay == WHITE_PIECE)
    {
        board->fullmoveNumber++;
    }
    
    
    int resetHalfMoveClock = 0;
    // to disable castling rights
    if (board->squares[move.fromSquare].type == KING || board->squares[move.fromSquare].type == ROOK)
    {
        int disableCastlingStartIndex = (board->squares[move.fromSquare].color == WHITE_PIECE) ? 0 : 2;
        if (board->squares[move.fromSquare].type == KING)
        {
            board->castlingRights[disableCastlingStartIndex] = 0;
            board->castlingRights[disableCastlingStartIndex + 1] = 0;
        }
        else
        {
            // rook move
            int queensideAddition = (move.fromSquare == 56 && board->squares[move.fromSquare].color == WHITE_PIECE) || (move.fromSquare == 0 && board->squares[move.fromSquare].color == BLACK_PIECE); // if the rook's column of its from square is 0, then queenside castling gets disabled
            if (queensideAddition || ((move.fromSquare == 63 && board->squares[move.fromSquare].color == WHITE_PIECE) || (move.fromSquare == 7 && board->squares[move.fromSquare].color == BLACK_PIECE)))
            {
                board->castlingRights[disableCastlingStartIndex + queensideAddition] = 0;
            }
        }
    }
    if (move.captureSquare.type == ROOK && (move.toSquare % 8 == 0 || move.toSquare % 8 == 7))
    {
        // rook capture
        int disableCastlingStartIndex = (board->squares[move.toSquare].color == WHITE_PIECE) ? 0 : 2;
        // int queensideAddition = (move.toSquare % 8) == 0; // if the rook's column of its from square is 0, then queenside castling gets disabled
        // board->castlingRights[disableCastlingStartIndex + queensideAddition] = 0;

        int queensideAddition = (move.toSquare == 56 && board->squares[move.toSquare].color == WHITE_PIECE) || (move.toSquare == 0 && board->squares[move.toSquare].color == BLACK_PIECE); // if the rook's column of its from square is 0, then queenside castling gets disabled
        if (queensideAddition || ((move.toSquare == 63 && board->squares[move.toSquare].color == WHITE_PIECE) || (move.toSquare == 7 && board->squares[move.toSquare].color == BLACK_PIECE)))
        {
            board->castlingRights[disableCastlingStartIndex + queensideAddition] = 0;
        }
    }

    board->enPassantSquareIndex = -1;

    if (move.isEnpassant == 1)
    {
        resetHalfMoveClock = 1;
        int pawnDirection;
        if (board->squares[move.fromSquare].color == WHITE_PIECE)
        {
            pawnDirection = 8;
        }
        else
        {
            pawnDirection = -8;
        }
        board->squares[move.toSquare + pawnDirection] = (Square){NONE, NONE, move.toSquare + pawnDirection};

        if (board->squares[move.fromSquare].color == WHITE_PIECE)
        {
            for (int j = 0; j < board->blackPieceAmt; j++)
            {
                if (board->blackPieceSquares[j].squareIndex == move.toSquare + pawnDirection)
                {
                    board->blackPieceSquares[j] = board->blackPieceSquares[board->blackPieceAmt - 1];
                    board->blackPieceAmt--;
                    break;
                }
            }
        }
        else
        {
            for (int j = 0; j < board->whitePieceAmt; j++)
            {
                if (board->whitePieceSquares[j].squareIndex == move.toSquare + pawnDirection)
                {
                    board->whitePieceSquares[j] = board->whitePieceSquares[board->whitePieceAmt - 1];
                    board->whitePieceAmt--;
                    break;
                }
            }
        }
    }
    else if (board->squares[move.fromSquare].type == PAWN)
    {
        resetHalfMoveClock = 1;
        if (board->squares[move.fromSquare].color == WHITE_PIECE)
        {
            if (move.toSquare == move.fromSquare - 16)
            {
                board->enPassantSquareIndex = move.fromSquare - 8;
            }
        }
        else
        {
            if (move.toSquare == move.fromSquare + 16)
            {
                board->enPassantSquareIndex = move.fromSquare + 8;
            }
        }
    }

    // for castling, moves the rook as well
    if (board->squares[move.fromSquare].type == KING)
    {
        // castling in this is represented by the king moving 2 squares at once
        int isCastlingMove = (move.fromSquare - 2 == move.toSquare || move.fromSquare + 2 == move.toSquare);
        if (isCastlingMove)
        {
            int rookIndexToMove;
            int rookDestinationIndexToMove;
            if (board->squares[move.fromSquare].color == WHITE_PIECE)
            {
                if (move.fromSquare - 2 == move.toSquare)
                {
                    // queenside for white
                    rookIndexToMove = 56;
                    rookDestinationIndexToMove = 59;
                }
                else
                {
                    // kingside for white
                    rookIndexToMove = 63;
                    rookDestinationIndexToMove = 61;
                }
            }
            else
            {
                if (move.fromSquare - 2 == move.toSquare)
                {
                    // queenside for black
                    rookIndexToMove = 0;
                    rookDestinationIndexToMove = 3;
                }
                else
                {
                    // kingside for black
                    rookIndexToMove = 7;
                    rookDestinationIndexToMove = 5;
                }
            }

            if (board->squares[move.fromSquare].color == WHITE_PIECE)
            {
                for (int i = 0; i < board->whitePieceAmt; i++)
                {
                    if (board->whitePieceSquares[i].squareIndex == rookIndexToMove)
                    {
                        // update the piece movement
                        board->whitePieceSquares[i].squareIndex = rookDestinationIndexToMove;
                        break;
                    }
                }
            }
            else
            {
                for (int i = 0; i < board->blackPieceAmt; i++)
                {
                    if (board->blackPieceSquares[i].squareIndex == rookIndexToMove)
                    {
                        // update the piece movement
                        board->blackPieceSquares[i].squareIndex = rookDestinationIndexToMove;
                        break;
                    }
                }
            }

            board->squares[rookDestinationIndexToMove] = board->squares[rookIndexToMove];
            board->squares[rookDestinationIndexToMove].squareIndex = rookDestinationIndexToMove;
            board->squares[rookIndexToMove] = (Square){NONE, NONE, rookIndexToMove};
        }
    }

    if (board->squares[move.fromSquare].color == WHITE_PIECE)
    {
        for (int i = 0; i < board->whitePieceAmt; i++)
        {
            if (board->whitePieceSquares[i].squareIndex == move.fromSquare)
            {
                // update the piece movement
                board->whitePieceSquares[i].squareIndex = move.toSquare;
                if (move.promotionType > PAWN)
                {
                    board->whitePieceSquares[i].type = move.promotionType;
                }

                if (move.captureSquare.type != NONE && !move.isEnpassant)
                {
                    resetHalfMoveClock = 1;
                    // there was a capture, must update other color's pieces
                    for (int j = 0; j < board->blackPieceAmt; j++)
                    {
                        if (board->blackPieceSquares[j].squareIndex == move.toSquare)
                        {
                            board->blackPieceSquares[j] = board->blackPieceSquares[board->blackPieceAmt - 1];
                            board->blackPieceAmt--;
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < board->blackPieceAmt; i++)
        {
            if (board->blackPieceSquares[i].squareIndex == move.fromSquare)
            {
                // update the piece movement
                board->blackPieceSquares[i].squareIndex = move.toSquare;
                if (move.promotionType > PAWN)
                {
                    board->blackPieceSquares[i].type = move.promotionType;
                }

                if (move.captureSquare.type != NONE && !move.isEnpassant)
                {
                    resetHalfMoveClock = 1;
                    // there was a capture, must update other color's pieces
                    for (int j = 0; j < board->whitePieceAmt; j++)
                    {
                        if (board->whitePieceSquares[j].squareIndex == move.toSquare)
                        {
                            board->whitePieceSquares[j] = board->whitePieceSquares[board->whitePieceAmt - 1];
                            board->whitePieceAmt--;
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    memcpy(&board->moves.stack[board->moves.size].oldMove, &move, sizeof(Move));
    board->moves.size++;

    board->squares[move.toSquare] = board->squares[move.fromSquare];
    board->squares[move.toSquare].squareIndex = move.toSquare;
    board->squares[move.fromSquare] = (Square){NONE, NONE, move.fromSquare};

    if (move.promotionType > PAWN)
    {
        board->squares[move.toSquare].type = move.promotionType;
        resetHalfMoveClock = 1;
    }

    if (resetHalfMoveClock == 1)
    {
        board->halfmoveClock = 0;
    } else {
        board->halfmoveClock++;
    }

    // printf("\nCurrent EP index: %d\n", board->enPassantSquareIndex);
    // printf("move %d to %d \n", move.fromSquare, move.toSquare);
    // printBoard(board);
}

void popMove(Board *board)
{
    if (board->moves.size == 0)
    {
        return;
    }

    // change the board turn
    board->colorToPlay = (board->colorToPlay == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
    if (board->colorToPlay == BLACK_PIECE)
    {
        board->fullmoveNumber--;
    }

    // first remove the move from the move stack
    UndoMove undoMove = board->moves.stack[board->moves.size - 1];
    board->moves.stack[board->moves.size - 1] = (UndoMove){0};

    board->moves.size--;

    board->enPassantSquareIndex = undoMove.oldEnPassantSquareIndex;

    memcpy(board->castlingRights, undoMove.oldCastlingRights, 4 * sizeof(int));

    memcpy(board->whitePieceSquares, undoMove.oldWhitePieceSquares, 16 * sizeof(Square));
    memcpy(board->blackPieceSquares, undoMove.oldBlackPieceSquares, 16 * sizeof(Square));
    board->whitePieceAmt = undoMove.oldWhitePieceAmt;
    board->blackPieceAmt = undoMove.oldBlackPieceAmt;
    board->gameState = undoMove.oldGameState;
    board->halfmoveClock = undoMove.oldHalfMoveClock;

    if (board->squares[undoMove.oldMove.toSquare].type == KING)
    {
        int isCastlingMove = (undoMove.oldMove.toSquare - 2 == undoMove.oldMove.fromSquare || undoMove.oldMove.toSquare + 2 == undoMove.oldMove.fromSquare);
        if (isCastlingMove)
        {
            int rookIndexToMove;
            int rookDestinationIndexToMove;

            if (board->squares[undoMove.oldMove.toSquare].color == WHITE_PIECE)
            {
                if (undoMove.oldMove.toSquare + 2 == undoMove.oldMove.fromSquare)
                {
                    // queenside for white
                    rookIndexToMove = 59;
                    rookDestinationIndexToMove = 56;
                }
                else
                {
                    // kingside for white
                    rookIndexToMove = 61;
                    rookDestinationIndexToMove = 63;
                }
            }
            else
            {
                if (undoMove.oldMove.toSquare + 2 == undoMove.oldMove.fromSquare)
                {
                    // queenside for black
                    rookIndexToMove = 3;
                    rookDestinationIndexToMove = 0;
                }
                else
                {
                    // kingside for black
                    rookIndexToMove = 5;
                    rookDestinationIndexToMove = 7;
                }
            }

            board->squares[rookDestinationIndexToMove] = board->squares[rookIndexToMove];
            board->squares[rookDestinationIndexToMove].squareIndex = rookDestinationIndexToMove;
            board->squares[rookIndexToMove] = (Square){NONE, NONE, rookIndexToMove};
        }
    }

    if (undoMove.oldMove.isEnpassant)
    {
        int pawnDirection = (board->squares[undoMove.oldMove.toSquare].color == WHITE_PIECE) ? 8 : -8;

        // set the from square to the place of the moved piece
        // note that to square is the empty square of where the piece actually went to
        board->squares[undoMove.oldMove.fromSquare] = board->squares[undoMove.oldMove.toSquare];
        board->squares[undoMove.oldMove.fromSquare].squareIndex = undoMove.oldMove.fromSquare;
        board->squares[undoMove.oldMove.toSquare] = (Square){NONE, NONE, undoMove.oldMove.toSquare};
        // now reset the piece which was captured
        board->squares[undoMove.oldMove.toSquare + pawnDirection] = undoMove.oldMove.captureSquare;
    }
    else
    {
        // set the from square to the place of the moved piece
        // then set the index back to the from square idex
        board->squares[undoMove.oldMove.fromSquare] = board->squares[undoMove.oldMove.toSquare];
        board->squares[undoMove.oldMove.fromSquare].squareIndex = undoMove.oldMove.fromSquare;

        if (undoMove.oldMove.captureSquare.type != NONE)
        {
            board->squares[undoMove.oldMove.toSquare] = undoMove.oldMove.captureSquare;
        }
        else
        {
            board->squares[undoMove.oldMove.toSquare] = (Square){NONE, NONE, undoMove.oldMove.toSquare};
        }
    }

    if (undoMove.oldMove.promotionType > PAWN)
    {
        board->squares[undoMove.oldMove.fromSquare].type = PAWN;
    }

    // printf("\nCurrent EP index: %d\n", board->enPassantSquareIndex);

    // printf("\n");
    // for (int i = 0; i<4; i++) {
    //     printf("%d ", board->castlingRights[i]);
    // }
    // printf("\n");
}

int isSlidingPiece(Square square)
{
    return (square.type == QUEEN || square.type == ROOK || square.type == BISHOP);
}

void initBoard(Board *board, char fen[], TranspositionTable *tt)
{
    for (int i = 0; i < 64; i++)
    {
        board->squares[i].type = NONE;
        board->squares[i].color = NONE;
        board->squares[i].squareIndex = i;
    }

    // printf("%s\n", fen);

    /*
    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

        Piece placement – rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR

        Active color – w or b

        Castling rights – KQkq (uppercase = white, lowercase = black)

        En passant target – e.g. e3 or -

        Halfmove clock – number of halfmoves since last capture or pawn move

        Fullmove number – move number starting from 1
    */

    int currentSquareIndex = 0;
    int fenReadIndex = 0;

    board->whitePieceAmt = 0;
    board->blackPieceAmt = 0;
    for (int i = 0; i < strlen(fen); i++)
    {

        if (fen[i] == '/')
        {
            continue;
        }
        else if (fen[i] == ' ')
        {
            fenReadIndex = i + 1;
            break;
        }

        if (isdigit(fen[i]))
        {
            currentSquareIndex += fen[i] - '0';
        }
        else
        {
            int pieceType, pieceColor;
            convertCharToPieceType(fen[i], &pieceType, &pieceColor);

            board->squares[currentSquareIndex].type = pieceType;
            board->squares[currentSquareIndex].color = pieceColor;
            board->squares[currentSquareIndex].squareIndex = currentSquareIndex;

            if (pieceColor == WHITE_PIECE)
            {
                board->whitePieceSquares[board->whitePieceAmt++] = board->squares[currentSquareIndex];
            }
            else
            {
                board->blackPieceSquares[board->blackPieceAmt++] = board->squares[currentSquareIndex];
            }
            currentSquareIndex++;
        }
    }

    // change the board turn
    if (fen[fenReadIndex] == 'w')
    {
        board->colorToPlay = WHITE_PIECE;
    }
    else
    {
        board->colorToPlay = BLACK_PIECE;
    }

    fenReadIndex += 2;
    char castlingRights[4];
    int castlingRightsIndex = 0;
    while (fen[fenReadIndex] != ' ')
    {
        castlingRights[castlingRightsIndex++] = fen[fenReadIndex++];
    }

    board->castlingRights[0] = 0;
    board->castlingRights[1] = 0;
    board->castlingRights[2] = 0;
    board->castlingRights[3] = 0;
    for (int i = 0; i < castlingRightsIndex; i++)
    {
        switch (castlingRights[i])
        {
        case 'K':
            board->castlingRights[0] = 1;
            break;
        case 'Q':
            board->castlingRights[1] = 1;
            break;
        case 'k':
            board->castlingRights[2] = 1;
            break;
        case 'q':
            board->castlingRights[3] = 1;
            break;
        };
    }

    fenReadIndex += 1;
    char enPassantField[2];
    int enPassantFieldIndex = 0;
    while (fen[fenReadIndex] != ' ')
    {
        enPassantField[enPassantFieldIndex++] = fen[fenReadIndex++];
    }

    board->enPassantSquareIndex = -1;
    if (enPassantFieldIndex > 1)
    {
        // turn something like f6 into the index 21
        board->enPassantSquareIndex = (enPassantField[0] - 'a') + ((8 - (enPassantField[1] - '0')) * 8);
    }

    fenReadIndex += 1;
    char halfmovesField[4];
    int halfmovesFieldIndex = 0;
    while (fen[fenReadIndex] != ' ')
    {
        halfmovesField[halfmovesFieldIndex++] = fen[fenReadIndex++];
    }

    fenReadIndex += 1;
    char fullmovesField[4];
    int fullmovesFieldIndex = 0;
    while (fen[fenReadIndex] != '\0')
    {
        fullmovesField[fullmovesFieldIndex++] = fen[fenReadIndex++];
    }

    board->halfmoveClock = atoi(halfmovesField);
    board->fullmoveNumber = atoi(fullmovesField);
    // printf("%d %d\n", board->halfmoveClock, board->fullmoveNumber);

    board->moves.size = 0;
    board->moves.capacity = 5;
    board->moves.stack = malloc(board->moves.capacity * sizeof(UndoMove));

    // printBoard(board);

    board->gameState = NONE;
    generateLegalMoves(board);
}
