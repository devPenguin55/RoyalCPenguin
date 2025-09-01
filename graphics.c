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
#include "evaluation.h"
#include "notations.h"

const int AI_COLOR = !BLACK_PIECE;
const int OPPONENT_COLOR = (AI_COLOR == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
const int AI_DEPTH = 6;

Sound sounds[6];
Texture2D spriteSheet;
Rectangle spriteRecs[12];
DrawingPieceMouseHandler drawingPieceMouseHandler;

void initGraphics(Texture2D *spriteSheet, Rectangle *spriteRecs, Sound *sounds)
{
    InitWindow(12 * 100 * 0.75, 8 * 100 * 0.75, "Chess Board");
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

void moveToNotation(Move *move, char *notation)
{
    int file = move->fromSquare % 8;       // 0 = 'a', ..., 7 = 'h'
    int rank = 7 - (move->fromSquare / 8); // 0 = '8', ..., 7 = '1'
    notation[0] = 'a' + file;
    notation[1] = '1' + rank;

    file = move->toSquare % 8;       // 0 = 'a', ..., 7 = 'h'
    rank = 7 - (move->toSquare / 8); // 0 = '8', ..., 7 = '1'
    notation[2] = 'a' + file;
    notation[3] = '1' + rank;
    notation[4] = '\0';
}

void drawArrow(Vector2 start, Vector2 end, float thickness, Color color)
{   
    
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float dist = sqrtf(dx*dx + dy*dy);
    float angle1 = atan2f(dy, dx);

    for (float d = 0; d <= dist; d += 3)
    {
        Vector2 pos = {
            start.x + cosf(angle1) * d,
            start.y + sinf(angle1) * d
        };
        DrawCircleV(pos, 9, color);
    }
    
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float arrowSize = 25.0;
    
    Vector2 left = {
        end.x - arrowSize * cosf(angle - PI/4),
        end.y - arrowSize * sinf(angle - PI/4)
    };
    dx = end.x - left.x;
    dy = end.y - left.y;
    dist = sqrtf(dx*dx + dy*dy);
    float angle2 = atan2f(dy, dx);
    for (float d = 0; d <= dist; d += 2)
    {
        Vector2 pos = {
            left.x + cosf(angle2) * d,
            left.y + sinf(angle2) * d
        };
        DrawCircleV(pos, 10, color);
    }

    Vector2 right = {
        end.x - arrowSize * cosf(angle + PI/4),
        end.y - arrowSize * sinf(angle + PI/4)
    };
    dx = end.x - right.x;
    dy = end.y - right.y;
    dist = sqrtf(dx*dx + dy*dy);
    float angle3 = atan2f(dy, dx);
    for (float d = 0; d <= dist; d += 2)
    {
        Vector2 pos = {
            right.x + cosf(angle3) * d,
            right.y + sinf(angle3) * d
        };
        DrawCircleV(pos, 10, color);
    }
}

void drawFrame(Board *board, Texture2D *spriteSheet, Rectangle *spriteRecs, DrawingPieceMouseHandler *drawingPieceMouseHandler, Sound *sounds, int showIndexes, LegalMovesContainer *curLegalMoves, TranspositionTable *tt, SearchRootResult *result, int *draggingPieceType, OpeningBook *book, char *notation)
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
            }
            else if (board->moves.size && board->moves.stack[board->moves.size - 1].oldMove.fromSquare == i + j * 8)
            {
                color = (Color){219, 180, 52, 255};
                colorAdjustment = 0;
            }
            else if (board->moves.size && board->moves.stack[board->moves.size - 1].oldMove.toSquare == i + j * 8)
            {
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

            if (j == 7)
            {
                DrawText(TextFormat("%c", "abcdefgh"[i]), i * 75, (j * 75) + 45, 30, BLACK);
            }

            if (i == 7)
            {
                DrawText(TextFormat("%c", "87654321"[j]), (i * 75) + 59, (j * 75), 30, BLACK);
            }

            if (kToDraw != -1)
            {
                snprintf(text, sizeof(text), "%d", kToDraw);
                DrawText(text, i * 75, j * 75 + 50, 30, WHITE);
            }
        }
    }

    AllPossibleOpeningMovesFromPosition possibleBookMoves = bookAllPossibleMoves(board, book);
    int maxOccurrence = 0;
    for (int i = 0; i<possibleBookMoves.amtMoves; i++){
        if (possibleBookMoves.moveOccurrence[i] > maxOccurrence) {
            maxOccurrence = possibleBookMoves.moveOccurrence[i];
        }
    }
    for (int moveIdx = 0; moveIdx<possibleBookMoves.amtMoves; moveIdx++) {
        Vector2 start = {
            (possibleBookMoves.moves[moveIdx].fromSquare % 8) * 75 + 75/2,
            (possibleBookMoves.moves[moveIdx].fromSquare / 8) * 75 + 75/2
        };
        Vector2 end = {
            (possibleBookMoves.moves[moveIdx].toSquare % 8) * 75 + 75/2,
            (possibleBookMoves.moves[moveIdx].toSquare / 8) * 75 + 75/2
        }; 

        if (start.x == end.x && start.y == end.y) {
            continue;
        }
        
        int occ = possibleBookMoves.moveOccurrence[moveIdx];
        float maxOcc = (double)(maxOccurrence);
        float t = (float)occ / maxOcc;
        if (t > 1.0f) t = 1.0f;
        Color low = (Color){ 255, 192, 203, 50 };   // pink
        Color high = (Color){ 128, 0, 255, 200 };   // purple-blue
        int r = (int)(low.r + (high.r - low.r) * t);
        int g = (int)(low.g + (high.g - low.g) * t);
        int b = (int)(low.b + (high.b - low.b) * t);
        int a = (int)(low.a + (high.a - low.a) * t);
        Color col = (Color){ r, g, b, a };

        drawArrow(start, end, 15.0, col);
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

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && (rand() % 100 == 0))
    {
        // undo the last 2 moves to undo ur move and the AI's move or in the other order for the opposite side to go again
        if (board->moves.size == 1) {
            popMove(board);
            WaitTime(0.75);
        } else {
            popMove(board);
            popMove(board);
        }
        WaitTime(0.25);
        *curLegalMoves = generateLegalMoves(board);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) // && board->colorToPlay == OPPONENT_COLOR)
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
                        if (curLegalMoves->moves[i].promotionType > PAWN)
                        {
                            if (curLegalMoves->moves[i].promotionType != QUEEN)
                            {
                                continue;
                            }
                        }
                        pushMove(board, curLegalMoves->moves[i]);
                        convertPieceTypeToTextureColumn(drawingPieceMouseHandler->squareSelected.type, draggingPieceType);
                        (*draggingPieceType) += drawingPieceMouseHandler->squareSelected.color * 6;
                        // printf("\n");
                        // for (int i = 0; i<4; i++) {
                        // printf("%d ", board->castlingRights[i]);
                        // }

                        *curLegalMoves = generateLegalMoves(board);
                        UndoMove lastMove = board->moves.stack[board->moves.size - 1];

                        if (board->gameState > CHECK)
                        {
                            PlaySound(sounds[4]);
                        }
                        else if (board->gameState == CHECK)
                        {
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

    

    if (board->gameState == CHECKMATE)
    {

        const char *text;
        if (board->colorToPlay == BLACK_PIECE)
        {
            text = "White  Won\nCheckmate!";
        }
        else
        {
            text = "Black  Won\nCheckmate!";
        }
        DrawText(text, 1 * 75, 3 * 75, 80, DARKGREEN);
    }
    else if (board->gameState == DRAW || board->gameState == DRAW_RETURN_0_IN_SEARCH)
    {
        char text[] = "DRAW!";
        DrawText(text, 2.5 * 75, 3.5 * 75, 80, DARKGREEN);
    } 

    // if (board->gameState <= CHECK)
    if (board->colorToPlay == AI_COLOR && board->gameState <= CHECK)
    {
        drawingPieceMouseHandler->isPickedUp = 0;

        // convertPieceTypeToTextureColumn(drawingPieceMouseHandler->squareSelected.type, &textureCol);

        SearchRootResult rootResult = IterativeDeepening(board, AI_DEPTH, tt, spriteSheet, spriteRecs, drawingPieceMouseHandler, &mousePosition, draggingPieceType, book, notation);
        memcpy(result, &rootResult, sizeof(SearchRootResult));
        pushMove(board, result->bestMove);
        
        *curLegalMoves = generateLegalMoves(board);
        
        
        UndoMove lastMove = board->moves.stack[board->moves.size - 1];
        
        if (board->gameState > CHECK)
        {
            PlaySound(sounds[4]);
        }
        else if (board->gameState == CHECK)
        {
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

        // uint64_t key = board->zobristHash;
        // Move *bestMoveFromTT = NULL;
        // TranspositionTableEntry *pEntry = &(tt->entries[key % tt->size]); // * pointer here avoids creating the object and taking more memory
        // if (pEntry->key == key) {
        //     // we have a match for the best move that was saved in the TT, will rank it highly
        //     bestMoveFromTT = &(pEntry->bestMove);
        // }
        // printf("\nBest Move %d to %d\n", bestMoveFromTT->fromSquare, bestMoveFromTT->toSquare);
    }
    else
    {
        // * graphics addition

        DrawRectangle(8 * 75, 0, 75 * 4, 75 * 8, DARKGRAY);
        char text[80];
        if (((result->bestScore > infinity - 500) || (result->bestScore < -infinity + 500)) && result->bestScore != UNKNOWN)
        {
            int whiteCenteredMateScore = (board->colorToPlay == WHITE_PIECE ? -1 : 1)*(infinity-abs(result->bestScore));
            snprintf(text, sizeof(text), "Searched depth %d\nEval:%s#%d", board->targetPly, (whiteCenteredMateScore >= 0) ? " \0" : " -\0", (abs(whiteCenteredMateScore)+1)/2);
        } else if (result->bestScore == UNKNOWN) {
            snprintf(text, sizeof(text), "Opening Book\nEval: Book Move");
        }
        else
        {
            double whiteCenteredEval = (board->colorToPlay == WHITE_PIECE ? -1 : 1)*((double)(result->bestScore))/((double)(pieceTypeToWorth[PAWN]));
            if (whiteCenteredEval == 0.0) {
                snprintf(text, sizeof(text), "Searched depth %d\nEval: 0.0", board->targetPly);
            } else {
                snprintf(text, sizeof(text), "Searched depth %d\nEval:%s%.1f", board->targetPly, (whiteCenteredEval >= 0) ? " +\0" : " \0", whiteCenteredEval);
            }
        }
        DrawText(text, 8 * 75 + 2, 2 * 75, 30, GREEN);
    
        DrawText("Bot Best Move\n---------------", 8 * 75 + 2, 4 * 75, 30, GREEN);
        DrawText(notation, 8 * 75 + 2, 5 * 75 - 15, 30, GREEN);

        EndDrawing();
    }
}
