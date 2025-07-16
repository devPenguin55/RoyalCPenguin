#ifndef BOARD_H
#define BOARD_H

#define WHITE_PIECE 0
#define BLACK_PIECE 1

#define NONE 0

#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

typedef struct Square {
    int type;
    int color;
    int squareIndex;
} Square;

typedef struct Move {
    int fromSquare;
    int toSquare;
    int promotionType;
    Square captureSquare;
    int isEnpassant;
} Move;

typedef struct UndoMove {
    int oldFromSquare;
    int oldToSquare;
    int oldPromotionType;
    Square oldCaptureSquare;
    int oldIsEnpassant;
    int oldEnPassantSquareIndex;
    int oldCastlingRights[4];
    int oldHalfMoveClock;
} UndoMove;

typedef struct MoveStack {
    UndoMove *stack; // pointer to the dynamic array
    int size; // current amt
    int capacity; // max stack before resizing
} MoveStack;

typedef struct Board {
    Square squares[64];
    MoveStack moves;
    int castlingRights[4]; // white kingside queenside, black kingside queenside
    int colorToPlay;
    int enPassantSquareIndex;
    int halfmoveClock; // number of halfmoves since pawn/capture
    int fullmoveNumber; // total fullmoves starting at 1
} Board;

void convertCharToPieceType(char pieceChar, int *pieceType, int *pieceColor);
void convertPieceTypeToChar(int pieceType, int pieceColor, char *pieceChar);
void printBoard(Board *board);
void printMove(Move move);
void pushMove(Board *board, Move move);
void popMove(Board *board);
int isSlidingPiece(Square square);
void initBoard(Board *board, char fen[]);

#endif