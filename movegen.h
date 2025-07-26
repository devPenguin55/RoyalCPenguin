#ifndef MOVEGEN_H
#define MOVEGEN_H

extern int numSquaresToEdge[64][20];
static const int pieceTypeToWorth[7] = {0, 1, 3, 3, 5, 9, 500}; // converts piece type to the worth, ex: KNIGHT = 2, so pieceTypeToWorth[2] is 3


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