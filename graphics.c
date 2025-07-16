#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <raylib.h>
#include <math.h>
#include "board.h"
#include "graphics.h"

Sound sounds[6];
Texture2D spriteSheet;
Rectangle spriteRecs[12];
DrawingPieceMouseHandler drawingPieceMouseHandler;


void initGraphics(Texture2D *spriteSheet, Rectangle *spriteRecs, Sound *sounds) {
    *spriteSheet = LoadTexture("chessPieces.png");
    SetTextureFilter(*spriteSheet, 1);


    int spriteWidth = spriteSheet->width / 6;
    int spriteHeight = spriteSheet->height / 2;

    for (int i = 0; i < 12; i++) { // 12 total sprites
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

void convertPieceTypeToTextureColumn(int pieceType, int *textureCol) {
    switch (pieceType) {
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

void drawFrame(Board *board, Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Sound *sounds, int showIndexes, LegalMovesContainer *curLegalMoves) {
    BeginDrawing();
    Color color;
    int colorAdjustment;
    ClearBackground(RAYWHITE);
    
    
    // Draw the background board
    for (int i = 0; i<8; i++) {
        for (int j = 0; j<8; j++) {
            if (drawingPieceMouseHandler->squareSelected.squareIndex == i+j*8) {
                color = (Color){230, 180, 52, 255};
                colorAdjustment = 0;
            } /*else if (board->moves.size && board->moves.stack[board->moves.size-1].fromSquare == i+j*8) {
                color = (Color){219, 180, 52, 255};  
                colorAdjustment = 0;
            } else if (board->moves.size && board->moves.stack[board->moves.size-1].toSquare == i+j*8) {
                color = (Color){200, 180, 52, 255};                  
                colorAdjustment = 0; 
            }*/ else if ((i+j) % 2 == 0) {
                color = (Color){240, 217, 181, 255}; // light
                colorAdjustment = -20;
            } else {
                color = (Color){181, 136, 99, 255}; // dark
                colorAdjustment = 0;
            }

            if (drawingPieceMouseHandler->squareSelected.color == board->colorToPlay) {
                for (int legalMoveIndex = 0; legalMoveIndex<curLegalMoves->amtOfMoves; legalMoveIndex++) {
                    if (curLegalMoves->moves[legalMoveIndex].fromSquare == drawingPieceMouseHandler->squareSelected.squareIndex && curLegalMoves->moves[legalMoveIndex].toSquare == i+j*8) {
                        color.r = 255 + colorAdjustment;  // stays high
                        color.g = 100 + colorAdjustment / 2;
                        color.b = 100 + colorAdjustment / 2;
                        break;
                    }
                } 
            }

            DrawRectangle(i*75, j*75, 75, 75, color);
            char text[3];
    
            snprintf(text, sizeof(text), "%d", i+j*8);
            if (showIndexes) {DrawText(text, i*75, j*75, 30, GREEN);}
        }
    }
    
    Square curSquare;
    int textureCol;
    for (int i = 0; i < 64; i++) {
        // get the piece on the square
        if (board->squares[i].type == NONE) {continue;}

        curSquare.type = board->squares[i].type;
        curSquare.color = board->squares[i].color;


        convertPieceTypeToTextureColumn(curSquare.type, &textureCol);
        

        // now draw it
        if (!(i == drawingPieceMouseHandler->squareSelected.squareIndex && drawingPieceMouseHandler->isPickedUp)) {
            Rectangle sourceRec = spriteRecs[textureCol+(curSquare.color*6)]; 
            Rectangle destRec = {
                75 * (i % 8), 75 * floor(i / 8), 
                sourceRec.width * 0.75 * 0.5, sourceRec.height * 0.75 * 0.5
            };
            DrawTexturePro(
                *spriteSheet, 
                sourceRec, 
                destRec,
                (Vector2){0, 0}, 
                0.0f,
                WHITE
            );
        }
    }

    // create the drag and drop for moving pieces
    Vector2 mousePosition = GetMousePosition();
    int snappedMouseX = (int)(floor(mousePosition.x / 75));
    int snappedMouseY = (int)(floor(mousePosition.y / 75));

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) { 
        popMove(board);
        *curLegalMoves = generateLegalMoves(board);
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (!drawingPieceMouseHandler->isPickedUp) {
            int squareInAvailablePieces = (board->squares[snappedMouseX+snappedMouseY*8].type != NONE) && (board->squares[snappedMouseX+snappedMouseY*8].color == board->colorToPlay);
            if (squareInAvailablePieces) {
                drawingPieceMouseHandler->squareSelected = board->squares[snappedMouseX+snappedMouseY*8];
                drawingPieceMouseHandler->isPickedUp = 1;
            }
        }
    } else {
        if (drawingPieceMouseHandler->isPickedUp == 1) {
            // was just placed down
            if (
                drawingPieceMouseHandler->squareSelected.squareIndex != -1 && 
                drawingPieceMouseHandler->squareSelected.squareIndex != snappedMouseX+snappedMouseY*8
            ) { 
                for (int i = 0; i<curLegalMoves->amtOfMoves; i++) {
                    if (
                        curLegalMoves->moves[i].toSquare == snappedMouseX+snappedMouseY*8 &&
                        curLegalMoves->moves[i].fromSquare == drawingPieceMouseHandler->squareSelected.squareIndex
                    ) {
                        pushMove(board, curLegalMoves->moves[i]);   
                        *curLegalMoves = generateLegalMoves(board);
                    
                        if (board->moves.stack[board->moves.size-1].oldCaptureSquare.type != NONE) {
                            PlaySound(sounds[1]);
                        } else {
                            PlaySound(sounds[0]);
                        }

                        printBoard(board);
                        break;
                    }
                }
            }
            drawingPieceMouseHandler->squareSelected = (Square){NONE, NONE, -1};
            drawingPieceMouseHandler->isPickedUp = 0;
        }
    }

    if (drawingPieceMouseHandler->isPickedUp) {
        convertPieceTypeToTextureColumn(drawingPieceMouseHandler->squareSelected.type, &textureCol);
        
        Rectangle destRec = {
            mousePosition.x-spriteRecs[textureCol+drawingPieceMouseHandler->squareSelected.color*6].width * 0.75 * 0.5 * 0.5, mousePosition.y-spriteRecs[textureCol+drawingPieceMouseHandler->squareSelected.color*6].height * 0.75 * 0.5 * 0.5, 
            spriteRecs[0].width * 0.75 * 0.5, spriteRecs[0].height * 0.75 * 0.5
        };
        
        DrawTexturePro(
            *spriteSheet, 
            spriteRecs[textureCol+drawingPieceMouseHandler->squareSelected.color*6], 
            destRec,
            (Vector2){0, 0}, 
            0.0f,
            WHITE
        );
    }

    


    EndDrawing();    
}
