#ifndef BOARD_H
#define BOARD_H
#include <stdint.h>

struct TranspositionTable; // forward delcare it to avoid circular includes

// piece color
#define WHITE_PIECE 0
#define BLACK_PIECE 1

// generic NONE for piece type and game state
#define NONE 0

// piece type
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

// game state
#define CHECK 1
#define CHECKMATE 2
#define DRAW 3

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct Square
{
    int type;
    int color;
    int squareIndex;
} Square;

typedef struct Move
{
    int fromSquare;
    int toSquare;
    int promotionType;
    Square captureSquare;
    int isEnpassant;
} Move;

typedef struct UndoMove
{
    Move oldMove;
    int oldEnPassantSquareIndex;
    int oldCastlingRights[4];
    int oldHalfMoveClock;
    Square oldWhitePieceSquares[16];
    int oldWhitePieceAmt;
    Square oldBlackPieceSquares[16];
    int oldBlackPieceAmt;
    int oldGameState;
    uint64_t oldZobristHash;
} UndoMove;

typedef struct MoveStack
{
    UndoMove *stack; // pointer to the dynamic array of undo moves
    int size;        // current amt
    int capacity;    // max stack before resizing
} MoveStack;

typedef struct Board
{
    Square squares[64];
    Square whitePieceSquares[16];
    int whitePieceAmt;
    Square blackPieceSquares[16];
    int blackPieceAmt;
    MoveStack moves;
    int castlingRights[4]; // white kingside queenside, black kingside queenside
    int colorToPlay;
    int enPassantSquareIndex;
    int halfmoveClock;  // number of halfmoves since pawn/capture
    int fullmoveNumber; // total fullmoves starting at 1
    int gameState;
    int targetPly;
    int materialScore;
    int pieceSquareTableScore;
} Board;

void convertCharToPieceType(char pieceChar, int *pieceType, int *pieceColor);
void convertPieceTypeToChar(int pieceType, int pieceColor, char *pieceChar);
void printBoard(Board *board);
void printMove(Move move);
void pushMove(Board *board, Move move);
void popMove(Board *board);
int isSlidingPiece(Square square);
void initBoard(Board *board, char fen[], struct TranspositionTable *tt);

#endif