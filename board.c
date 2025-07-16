#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "board.h"
#include "movegen.h"

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
    // change the board turn
    board->colorToPlay = (board->colorToPlay == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;

    if (board->colorToPlay == WHITE_PIECE) {
        board->fullmoveNumber++;
    }

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

    board->moves.stack[board->moves.size].oldEnPassantSquareIndex = board->enPassantSquareIndex;
    board->moves.stack[board->moves.size].oldCastlingRights[0] = board->castlingRights[0]; 
    board->moves.stack[board->moves.size].oldCastlingRights[1] = board->castlingRights[1]; 
    board->moves.stack[board->moves.size].oldCastlingRights[2] = board->castlingRights[2]; 
    board->moves.stack[board->moves.size].oldCastlingRights[3] = board->castlingRights[3]; 
    board->moves.stack[board->moves.size].oldHalfMoveClock = board->halfmoveClock;


    if (board->squares[move.fromSquare].type == KING || board->squares[move.fromSquare].type == ROOK) {
        int disableCastlingStartIndex = (board->squares[move.fromSquare].color == WHITE_PIECE) ? 0 : 2;
        if (board->squares[move.fromSquare].type == KING) {
            board->castlingRights[disableCastlingStartIndex] = 0;
            board->castlingRights[disableCastlingStartIndex+1] = 0;
        } else {
            // rook move
            int queensideAddition = (move.fromSquare % 8) == 0; // if the rook's column of its from square is 0, then queenside castling gets disabled
            board->castlingRights[disableCastlingStartIndex + queensideAddition] = 0;
        }
    }
    if (move.captureSquare.type == ROOK && (move.toSquare % 8 == 0 || move.toSquare % 8 == 7)) {
        // rook capture
        int disableCastlingStartIndex = (board->squares[move.toSquare].color == WHITE_PIECE) ? 0 : 2;
        int queensideAddition = (move.toSquare % 8) == 0; // if the rook's column of its from square is 0, then queenside castling gets disabled
        board->castlingRights[disableCastlingStartIndex + queensideAddition] = 0;
    }

    board->enPassantSquareIndex = -1;

    if (move.isEnpassant == 1) {
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
    } else if (board->squares[move.fromSquare].type == PAWN) {        
        if (board->squares[move.fromSquare].color == WHITE_PIECE)
        {
            if (move.toSquare == move.fromSquare-16)
            {
                board->enPassantSquareIndex = move.fromSquare-8;
            }
        }
        else
        {
            if (move.toSquare == move.fromSquare+16)
            {
                board->enPassantSquareIndex = move.fromSquare+8;
            } 
        }
    }

    board->moves.stack[board->moves.size].oldFromSquare = move.fromSquare;
    board->moves.stack[board->moves.size].oldToSquare = move.toSquare;
    board->moves.stack[board->moves.size].oldPromotionType = move.promotionType;
    board->moves.stack[board->moves.size].oldCaptureSquare = move.captureSquare;
    board->moves.stack[board->moves.size].oldIsEnpassant = move.isEnpassant;

    board->moves.size++;

    // for castling
    if (board->squares[move.fromSquare].type == KING) {
        // castling in this is represented by the king moving 2 squares at once
        int isCastlingMove = (move.fromSquare - 2 == move.toSquare || move.fromSquare + 2 == move.toSquare);
        if (isCastlingMove) {
            printf("CASTLING\n");
            int rookIndexToMove;
            int rookDestinationIndexToMove;
            if (board->squares[move.fromSquare].color == WHITE_PIECE) {
                if (move.fromSquare - 2 == move.toSquare) {
                    // queenside for white
                    rookIndexToMove = 56;
                    rookDestinationIndexToMove = 59;
                } else {
                    // kingside for white
                    rookIndexToMove = 63;
                    rookDestinationIndexToMove = 61;
                }
            } else {
                if (move.fromSquare - 2 == move.toSquare) {
                    // queenside for black
                    rookIndexToMove = 0;
                    rookDestinationIndexToMove = 3;
                } else {
                    // kingside for black
                    rookIndexToMove = 7;
                    rookDestinationIndexToMove = 5;
                }
            }

            board->squares[rookDestinationIndexToMove] = board->squares[rookIndexToMove];
            board->squares[rookDestinationIndexToMove].squareIndex = rookDestinationIndexToMove;
            board->squares[rookIndexToMove] = (Square){NONE, NONE, rookIndexToMove};
        }
    }


    board->squares[move.toSquare] = board->squares[move.fromSquare];
    board->squares[move.toSquare].squareIndex = move.toSquare;
    board->squares[move.fromSquare] = (Square){NONE, NONE, move.fromSquare};
    

    // printf("\nCurrent EP index: %d\n", board->enPassantSquareIndex);
}

void popMove(Board *board)
{
    if (board->moves.size == 0)
    {
        return;
    }

    // change the board turn
    board->colorToPlay = (board->colorToPlay == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
    if (board->colorToPlay == BLACK_PIECE) {
        board->fullmoveNumber--;
    }

    // first remove the move from the move stack
    UndoMove undoMove = board->moves.stack[board->moves.size - 1];
    board->moves.stack[board->moves.size - 1] = (UndoMove){-1, -1, -1, (Square){NONE, NONE, -1}, 0, -1, {-1, -1, -1, -1}, -1};

    board->moves.size--;

    board->enPassantSquareIndex = undoMove.oldEnPassantSquareIndex;
    board->castlingRights[0] = undoMove.oldCastlingRights[0];
    board->castlingRights[1] = undoMove.oldCastlingRights[1];
    board->castlingRights[2] = undoMove.oldCastlingRights[2];
    board->castlingRights[3] = undoMove.oldCastlingRights[3];

    if (board->squares[undoMove.oldToSquare].type == KING) {
        int isCastlingMove = (undoMove.oldToSquare - 2 == undoMove.oldFromSquare || undoMove.oldToSquare + 2 == undoMove.oldFromSquare);
        if (isCastlingMove) {
            int rookIndexToMove;
            int rookDestinationIndexToMove;
            
            if (board->squares[undoMove.oldToSquare].color == WHITE_PIECE) {
                if (undoMove.oldToSquare + 2 == undoMove.oldFromSquare) {
                    // queenside for white
                    rookIndexToMove = 59;
                    rookDestinationIndexToMove = 56;
                } else {
                    // kingside for white
                    rookIndexToMove = 61;
                    rookDestinationIndexToMove = 63;
                }
            } else {
                if (undoMove.oldToSquare + 2 == undoMove.oldFromSquare) {
                    // queenside for black
                    rookIndexToMove = 3;
                    rookDestinationIndexToMove = 0;
                } else {
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

    if (undoMove.oldIsEnpassant)
    {
        int pawnDirection = (board->squares[undoMove.oldFromSquare].color == WHITE_PIECE) ? 8 : -8;
        if (board->squares[undoMove.oldFromSquare].color == WHITE_PIECE)

        // set the from square to the place of the moved piece
        // note that to square is the empty square of where the piece actually went to
        board->squares[undoMove.oldFromSquare] = board->squares[undoMove.oldToSquare];
        board->squares[undoMove.oldFromSquare].squareIndex = undoMove.oldFromSquare;
        board->squares[undoMove.oldToSquare] = (Square){NONE, NONE, undoMove.oldToSquare};
        // now reset the piece which was captured
        board->squares[undoMove.oldToSquare + pawnDirection] = undoMove.oldCaptureSquare;
    }
    else
    {
        // set the from square to the place of the moved piece
        // then set the index back to the from square idex
        board->squares[undoMove.oldFromSquare] = board->squares[undoMove.oldToSquare];
        board->squares[undoMove.oldFromSquare].squareIndex = undoMove.oldFromSquare;

        if (undoMove.oldCaptureSquare.type != NONE)
        {
            board->squares[undoMove.oldToSquare] = undoMove.oldCaptureSquare;
        }
        else
        {
            board->squares[undoMove.oldToSquare] = (Square){NONE, NONE, undoMove.oldToSquare};
        }
    }

    // for (int i = 0; i < 499999999; i++)
    // {
    //     i*i;
    // }
    // printf("\nCurrent EP index: %d\n", board->enPassantSquareIndex);
}

int isSlidingPiece(Square square)
{
    return (square.type == QUEEN || square.type == ROOK || square.type == BISHOP);
}

void initBoard(Board *board, char fen[])
{
    for (int i = 0; i < 64; i++)
    {
        board->squares[i].type = NONE;
        board->squares[i].color = NONE;
        board->squares[i].squareIndex = i;
    }

    printf("%s\n", fen);

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
    int enPassantDetectingIndexToSkip = -1;
    if (enPassantFieldIndex > 1)
    {
        // turn something like f6 into the index 21
        board->enPassantSquareIndex = (enPassantField[0] - 'a') + ((8 - (enPassantField[1] - '0')) * 8);
        enPassantDetectingIndexToSkip = board->enPassantSquareIndex;
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
    printf("%d %d %d\n", board->halfmoveClock, board->fullmoveNumber);

    
    board->moves.size = 0;
    board->moves.capacity = 5;
    board->moves.stack = malloc(board->moves.capacity * sizeof(UndoMove));

    printBoard(board);
}
