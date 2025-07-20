#ifndef MOVEGEN_H
#define MOVEGEN_H

extern int numSquaresToEdge[64][20];

typedef struct LegalMovesContainer
{
    Move *moves;
    int amtOfMoves;
} LegalMovesContainer;

void fillNumSquaresToEdge();
Square generateAttackingSquares(Board *board, int colorOfFromPieceToAttack);
LegalMovesContainer generatePseudoLegalMoves(Board *board);
LegalMovesContainer generateLegalMoves(Board *board);
int moveGenerationTest(Board *board, int depth);
void initMoveGen();

#endif