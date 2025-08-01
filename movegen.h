#ifndef MOVEGEN_H
#define MOVEGEN_H

extern int numSquaresToEdge[64][20];
static const int pieceTypeToWorth[7] = {0, 1, 3, 3, 5, 9, 500}; // converts piece type to the worth, ex: KNIGHT = 2, so pieceTypeToWorth[2] is 3
static const int FLAG_EXACT = 0;
static const int FLAG_ALPHA = 1;
static const int FLAG_BETA  = 2;
static const long long UNKNOWN = -999999999; 
static const int infinity   =       1000000;

struct TranspositionTable; // forward delcare it to avoid circular includes

typedef struct LegalMovesContainer
{
    Move *moves;
    int amtOfMoves;
} LegalMovesContainer;

void fillNumSquaresToEdge();
int kingIsAttacked(Board *board, int colorToCheck);
LegalMovesContainer generatePseudoLegalMoves(Board *board);
LegalMovesContainer generateLegalMoves(Board *board);
int moveGenerationTest(Board *board, int depth);
void initMoveGen();

#endif