#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <raylib.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "graphics.h"
#include "search.h"
#include "zobrist.h"

const int AI_COLOR = WHITE_PIECE;
const int OPPONENT_COLOR = (AI_COLOR == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
const int AI_DEPTH = 6;

Sound sounds[6];
Texture2D spriteSheet;
Rectangle spriteRecs[12];
DrawingPieceMouseHandler drawingPieceMouseHandler;

void initGraphics(Texture2D *spriteSheet, Rectangle *spriteRecs, Sound *sounds)
{
    InitWindow(8*100*0.75, 8*100*0.75, "Chess Board");
    InitAudioDevice();
    
    *spriteSheet = LoadTexture("chessPieces.png");
    SetTextureFilter(*spriteSheet, 1);

    int spriteWidth = spriteSheet->width / 6;
    int spriteHeight = spriteSheet->height / 2;

    for (int i = 0; i < 12; i++)
    { // 12 total sprites
        int row = i / 6;
        int col = i % 6;

        spriteRecs[i].x = col * spriteWidth;
        spriteRecs[i].y = row * spriteHeight;
        spriteRecs[i].width = spriteWidth;
        spriteRecs[i].height = spriteHeight;
    }

    sounds[0] = LoadSound("move.mp3");
    sounds[1] = LoadSound("capture.mp3");
    sounds[2] = LoadSound("check.mp3");
    sounds[3] = LoadSound("castle.mp3");
    sounds[4] = LoadSound("checkmate.mp3");
    sounds[5] = LoadSound("promote.mp3");

    drawingPieceMouseHandler.isPickedUp = 0;
    drawingPieceMouseHandler.squareSelected = (Square){NONE, NONE, -1};
}

void convertPieceTypeToTextureColumn(int pieceType, int *textureCol)
{
    switch (pieceType)
    {
    case KING:
        *textureCol = 0;
        break;
    case QUEEN:
        *textureCol = 1;
        break;
    case BISHOP:
        *textureCol = 2;
        break;
    case KNIGHT:
        *textureCol = 3;
        break;
    case ROOK:
        *textureCol = 4;
        break;
    case PAWN:
        *textureCol = 5;
        break;
    }
}

void drawFrame(Board *board, Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Sound *sounds, int showIndexes, LegalMovesContainer *curLegalMoves, TranspositionTable *tt)
{
    BeginDrawing();
    
    Color color;
    int colorAdjustment;
    ClearBackground(DARKGRAY);

    // Draw the background board
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (drawingPieceMouseHandler->squareSelected.squareIndex == i + j * 8)
            {
                color = (Color){230, 180, 52, 255};
                colorAdjustment = 0;
            } else if (board->moves.size && board->moves.stack[board->moves.size-1].oldMove.fromSquare == i+j*8) {
                color = (Color){219, 180, 52, 255};
                colorAdjustment = 0;
            } else if (board->moves.size && board->moves.stack[board->moves.size-1].oldMove.toSquare == i+j*8) {
                color = (Color){200, 180, 52, 255};
                colorAdjustment = 0;
            }
            else if ((i + j) % 2 == 0)
            {
                color = (Color){240, 217, 181, 255}; // light
                colorAdjustment = -20;
            }
            else
            {
                color = (Color){181, 136, 99, 255}; // dark
                colorAdjustment = 0;
            }

            if (drawingPieceMouseHandler->squareSelected.color == board->colorToPlay)
            {
                for (int legalMoveIndex = 0; legalMoveIndex < curLegalMoves->amtOfMoves; legalMoveIndex++)
                {
                    if (curLegalMoves->moves[legalMoveIndex].fromSquare == drawingPieceMouseHandler->squareSelected.squareIndex && curLegalMoves->moves[legalMoveIndex].toSquare == i + j * 8)
                    {
                        color.r = 255 + colorAdjustment; // stays high
                        color.g = 100 + colorAdjustment / 2;
                        color.b = 100 + colorAdjustment / 2;
                        colorAdjustment = -60;
                        break;
                    }
                }
            }

            int kToDraw = -1;
            // if (board->colorToPlay == BLACK_PIECE) {
            //     for (int k = 0; k < board->whiteAttackingAmt; k++)
            //     {
            //         if (i + j * 8 == board->whiteAttackingSquares[k].attackingSquare.squareIndex)
            //         {
            //             color.r = 60 + colorAdjustment * 2; // stays high
            //             color.g = 175 + colorAdjustment;
            //             color.b = 217 + colorAdjustment;
            //             kToDraw = k;
            //             break;
            //         }
            //     }
            // } else {
            //     for (int k = 0; k < board->blackAttackingAmt; k++)
            //     {
            //         if (i + j * 8 == board->blackAttackingSquares[k].attackingSquare.squareIndex)
            //         {
            //             color.r = 80 + colorAdjustment; // stays high
            //             color.g = 200 + colorAdjustment;
            //             color.b = 126 + colorAdjustment;
            //             kToDraw = k;
            //             break;
            //         }
            //     }
            // }

            DrawRectangle(i * 75, j * 75, 75, 75, color);

            char text[3];
            snprintf(text, sizeof(text), "%d", i + j * 8);
            if (showIndexes)
            {
                DrawText(text, i * 75, j * 75, 30, GREEN);
            }

            if (kToDraw != -1)
            {
                snprintf(text, sizeof(text), "%d", kToDraw);
                DrawText(text, i * 75, j * 75 + 50, 30, WHITE);
            }
        }
    }

    Square curSquare;
    int textureCol;
    for (int i = 0; i < 64; i++)
    {
        // get the piece on the square
        if (board->squares[i].type == NONE)
        {
            continue;
        }

        curSquare.type = board->squares[i].type;
        curSquare.color = board->squares[i].color;

        convertPieceTypeToTextureColumn(curSquare.type, &textureCol);

        // now draw it
        if (!(i == drawingPieceMouseHandler->squareSelected.squareIndex && drawingPieceMouseHandler->isPickedUp))
        {
            Rectangle sourceRec = spriteRecs[textureCol + (curSquare.color * 6)];
            Rectangle destRec = {
                75 * (i % 8), 75 * floor(i / 8),
                sourceRec.width * 0.75 * 0.5, sourceRec.height * 0.75 * 0.5};
            DrawTexturePro(
                *spriteSheet,
                sourceRec,
                destRec,
                (Vector2){0, 0},
                0.0f,
                WHITE);
        }
    }

    // create the drag and drop for moving pieces
    Vector2 mousePosition = GetMousePosition();
    int snappedMouseX = (int)(floor(mousePosition.x / 75));
    int snappedMouseY = (int)(floor(mousePosition.y / 75));

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        // undo the last 2 moves to undo ur move and the AI's move or in the other order for the opposite side to go again
        popMove(board);
        popMove(board);
        WaitTime(0.25);
        *curLegalMoves = generateLegalMoves(board);        
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))// && board->colorToPlay == OPPONENT_COLOR)
    {
        if (!drawingPieceMouseHandler->isPickedUp)
        {
            int squareInAvailablePieces = (board->squares[snappedMouseX + snappedMouseY * 8].type != NONE) && (board->squares[snappedMouseX + snappedMouseY * 8].color == board->colorToPlay);
            if (squareInAvailablePieces)
            {
                drawingPieceMouseHandler->squareSelected = board->squares[snappedMouseX + snappedMouseY * 8];
                drawingPieceMouseHandler->isPickedUp = 1;
            }
        }
    }
    else
    {
        if (drawingPieceMouseHandler->isPickedUp == 1)
        {
            // was just placed down
            if (
                drawingPieceMouseHandler->squareSelected.squareIndex != -1 &&
                drawingPieceMouseHandler->squareSelected.squareIndex != snappedMouseX + snappedMouseY * 8)
            {
                for (int i = 0; i < curLegalMoves->amtOfMoves; i++)
                {
                    if (
                        curLegalMoves->moves[i].toSquare == snappedMouseX + snappedMouseY * 8 &&
                        curLegalMoves->moves[i].fromSquare == drawingPieceMouseHandler->squareSelected.squareIndex)
                    {
                        if (curLegalMoves->moves[i].promotionType > PAWN) {
                            if (curLegalMoves->moves[i].promotionType != QUEEN) {
                                continue;
                            }
                        }
                        pushMove(board, curLegalMoves->moves[i]);
                        
                        // printf("\n");
                        // for (int i = 0; i<4; i++) {
                                // printf("%d ", board->castlingRights[i]);
                            // }
                            
                        *curLegalMoves = generateLegalMoves(board);
                        UndoMove lastMove = board->moves.stack[board->moves.size - 1];
                        
                        if (board->gameState > CHECK) {
                            PlaySound(sounds[4]);
                        }
                        else if (board->gameState == CHECK) {
                            PlaySound(sounds[2]);
                        }
                        else if (board->squares[lastMove.oldMove.toSquare].type == KING && ((lastMove.oldMove.toSquare - 2 == lastMove.oldMove.fromSquare) || (lastMove.oldMove.toSquare + 2 == lastMove.oldMove.fromSquare)))
                        {
                            PlaySound(sounds[3]);
                        }
                        else if (lastMove.oldMove.captureSquare.type != NONE)
                        {
                            PlaySound(sounds[1]);
                        }
                        else
                        {
                            PlaySound(sounds[0]);
                        }

                        // printBoard(board);
                        break;
                    }
                }
            }
            drawingPieceMouseHandler->squareSelected = (Square){NONE, NONE, -1};
            drawingPieceMouseHandler->isPickedUp = 0;
        }
    }

    if (drawingPieceMouseHandler->isPickedUp)
    {
        convertPieceTypeToTextureColumn(drawingPieceMouseHandler->squareSelected.type, &textureCol);

        Rectangle destRec = {
            mousePosition.x - spriteRecs[textureCol + drawingPieceMouseHandler->squareSelected.color * 6].width * 0.75 * 0.5 * 0.5, mousePosition.y - spriteRecs[textureCol + drawingPieceMouseHandler->squareSelected.color * 6].height * 0.75 * 0.5 * 0.5,
            spriteRecs[0].width * 0.75 * 0.5, spriteRecs[0].height * 0.75 * 0.5};

        DrawTexturePro(
            *spriteSheet,
            spriteRecs[textureCol + drawingPieceMouseHandler->squareSelected.color * 6],
            destRec,
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }

    if (board->gameState == CHECKMATE) {

        const char *text;
        if (board->colorToPlay == BLACK_PIECE) {
            text = "White  Won\nCheckmate!";
        } else {
            text = "Black  Won\nCheckmate!";
        }
        DrawText(text, 1 * 75, 3 * 75, 80, DARKGREEN);

    } else if (board->gameState == STALEMATE) {
        char text[] = "Stalemate!";
        DrawText(text, 1 * 75, 3.5 * 75, 80, DARKGREEN);
    }

    if (board->colorToPlay == AI_COLOR && board->gameState <= CHECK) {
        drawingPieceMouseHandler->isPickedUp = 0;
        pushMove(board, IterativeDeepening(board, AI_DEPTH, tt));
        *curLegalMoves = generateLegalMoves(board);

        UndoMove lastMove = board->moves.stack[board->moves.size - 1];

        if (board->gameState > CHECK) {
            PlaySound(sounds[4]);
        }
        else if (board->gameState == CHECK) {
            PlaySound(sounds[2]);
        }
        else if (board->squares[lastMove.oldMove.toSquare].type == KING && ((lastMove.oldMove.toSquare - 2 == lastMove.oldMove.fromSquare) || (lastMove.oldMove.toSquare + 2 == lastMove.oldMove.fromSquare)))
        {
            PlaySound(sounds[3]);
        }
        else if (lastMove.oldMove.captureSquare.type != NONE)
        {
            PlaySound(sounds[1]);
        }
        else
        {
            PlaySound(sounds[0]);
        }
    }


    // char text3[64];
    // snprintf(text3, sizeof(text3), "%llu", );
    // DrawText(text3, 8.5 * 75, 4 * 75, 20, WHITE);

    EndDrawing();
}
