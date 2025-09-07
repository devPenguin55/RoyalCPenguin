#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "movegen.h"
#include "notations.h"


void convertSquareIndexToAlgebraicForm(int squareIndex, char *out) {
    int file = squareIndex % 8; // abcdefgh
    int rank = 7-(squareIndex / 8); // 12345678

    char files[] = "abcdefgh";
    char ranks[] = "12345678";

    out[0] = files[file];
    out[1] = ranks[rank];
    out[2] = '\0';
}

void convertMoveToSAN(Board *board, Move move, char *out) {
    if (move.fromSquare == -1) {
        strcpy(out, "No Move\n(Didn't Search)");
        return;
    }


    char fromSquare[3];
    char toSquare[3];
    convertSquareIndexToAlgebraicForm(move.fromSquare, fromSquare);
    convertSquareIndexToAlgebraicForm(move.toSquare, toSquare);
    
    strcpy(out, "");
    if (board->squares[move.fromSquare].type == KING && (move.fromSquare - 2 == move.toSquare || move.fromSquare + 2 == move.toSquare)) {
        if (
            (board->squares[move.fromSquare].color == WHITE_PIECE && move.fromSquare + 2 == move.toSquare) ||
            (board->squares[move.fromSquare].color == BLACK_PIECE && move.fromSquare + 2 == move.toSquare) 
        ) {
            strcpy(out, "O-O");
        } else {
            strcpy(out, "O-O-O");
        }
    } else if (board->squares[move.fromSquare].type == PAWN)
    {
        out[0] = fromSquare[0];
        if (move.captureSquare.type == NONE) {
            out[1] = toSquare[1];
            if (move.promotionType != -1) {
                char promotionChar;
                convertPieceTypeToChar(move.promotionType, WHITE_PIECE, &promotionChar);
                out[2] = '=';
                out[3] = promotionChar;
                out[4] = '\0';
            } else {
                out[2] = '\0';
            }
        } else {
            out[1] = 'x';
            out[2] = toSquare[0];
            out[3] = toSquare[1];
            if (move.promotionType != -1) {
                char promotionChar;
                convertPieceTypeToChar(move.promotionType, WHITE_PIECE, &promotionChar);
                out[4] = '=';
                out[5] = promotionChar;
                out[6] = '\0';
            } else {
                out[4] = '\0';
            }
        }
    } else {
        char pieceMovingChar;
        convertPieceTypeToChar(board->squares[move.fromSquare].type, WHITE_PIECE, &pieceMovingChar);
        out[0] = pieceMovingChar;
        if (move.captureSquare.type == NONE) {
            out[1] = toSquare[0];
            out[2] = toSquare[1];
            out[3] = '\0';
        } else {
            out[1] = 'x';
            out[2] = toSquare[0];
            out[3] = toSquare[1];
            out[4] = '\0';
        }
    }

    
    pushMove(board, move);
    LegalMovesContainer opponentResponseMoves = generateLegalMoves(board);
    if (board->gameState == CHECK) {
        strcat(out, "+");
    } else if (board->gameState == CHECKMATE) {
        strcat(out, "#");
    }
    free(opponentResponseMoves.moves);
    popMove(board);
    
    
    LegalMovesContainer legalMoves = generateLegalMoves(board);
    for (int i = 0; i < legalMoves.amtOfMoves; i++)
    {
        if (
            (board->squares[legalMoves.moves[i].fromSquare].type == board->squares[move.fromSquare].type) &&
            (board->squares[legalMoves.moves[i].fromSquare].color == board->squares[move.fromSquare].color) &&
            (legalMoves.moves[i].fromSquare != move.fromSquare && legalMoves.moves[i].toSquare == move.toSquare) &&
            (board->squares[move.fromSquare].type != PAWN)

        ) {
            // handle the disambiguity
            if ((legalMoves.moves[i].fromSquare % 8) == (move.fromSquare % 8)) {
                // same file
                size_t len = strlen(out);
                for (int i = len; i >= 1; i--) {
                    out[i+1] = out[i]; 
                }
                out[1] = fromSquare[0];
            } else if ((legalMoves.moves[i].fromSquare / 8) == (move.fromSquare / 8)) {
                // same rank
                size_t len = strlen(out);
                for (int i = len; i >= 1; i--) {
                    out[i+1] = out[i]; 
                }
                out[1] = fromSquare[0];
            } else {
                // diff rank and file, js put file first (that's what chess websites do)
                size_t len = strlen(out);
                for (int i = len; i >= 1; i--) {
                    out[i+1] = out[i]; 
                }
                out[1] = fromSquare[0];
            }
            break;
        }
    }
    free(legalMoves.moves);
    
    return;
}

void convertMoveToUCI(Board *board, Move move, char *out) {
    char fromSquare[3];
    char toSquare[3];
    convertSquareIndexToAlgebraicForm(move.fromSquare, fromSquare);
    convertSquareIndexToAlgebraicForm(move.toSquare, toSquare);
    out[0] = fromSquare[0];
    out[1] = fromSquare[1];
    out[2] = toSquare[0];
    out[3] = toSquare[1];

    if (move.promotionType != -1) {
        char promotionType;
        convertPieceTypeToChar(move.promotionType, BLACK_PIECE, &promotionType);
        out[4] = promotionType;
        out[5] = '\0';
    } else {
        out[4] = '\0';
    }
}

void pushUCIToBoard(Board *board, char *uci) {
    LegalMovesContainer legalMoves = generateLegalMoves(board);

    for (int i = 0; i<legalMoves.amtOfMoves; i++) {
        char out[6];
        char fromSquare[3];
        char toSquare[3];
        convertSquareIndexToAlgebraicForm(legalMoves.moves[i].fromSquare, fromSquare);
        convertSquareIndexToAlgebraicForm(legalMoves.moves[i].toSquare, toSquare);
        out[0] = fromSquare[0];
        out[1] = fromSquare[1];
        out[2] = toSquare[0];
        out[3] = toSquare[1];

        if (legalMoves.moves[i].promotionType != NONE) {
            char promotionType;
            convertPieceTypeToChar(legalMoves.moves[i].promotionType, BLACK_PIECE, &promotionType);
            out[4] = promotionType;
            out[5] = '\0';
        } else {
            out[4] = '\0';
        }

        if (strcmp(uci, out) == 0) {
            pushMove(board, legalMoves.moves[i]);
            free(legalMoves.moves);
            return;
        }
    }

    free(legalMoves.moves);
}