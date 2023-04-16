#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    const int screenWidth = 800;
    const int screenHeight = 450;

    int px = screenWidth/2;
    int py = screenHeight/2;

    bool isFuelSpawned = false;
    
    bool isPlaying = true;

    double g = 0;

    double fuel = 100.0;

    int fuelValue = 10;

    int score = 0;

    Rectangle playerRect = { px, py, 20, 20 }; // player
    Rectangle fuelRect = { GetRandomValue(0, screenWidth - 20), GetRandomValue(0, screenHeight - 20), 20, 20 }; // fuel
    Rectangle fuelBarRect = { 15,15,20,100 };
    Rectangle fuelBarBackground = { 10,10,30,110 };
    float radius = 10;

    InitWindow(screenWidth, screenHeight, "jetpack fella");

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_RIGHT)){
            px += 2;
        }
        if (IsKeyDown(KEY_LEFT)){
            px -= 2;
        }
        if (IsKeyDown(KEY_SPACE) && fuel > 0){
            g = 4;
            fuel -= 0.5;
            DrawText("*", px, py+10, 20, RED);
        }

        playerRect.x = px;
        playerRect.y = py;

        if (CheckCollisionRecs(playerRect, fuelRect)) {
            if (fuel + fuelValue <= 100) {
                fuel += fuelValue;
            } else if (fuel + fuelValue >= 100) {
                fuel = 100;
            }
            isFuelSpawned = false;
            score++;
        }

        BeginDrawing();

            ClearBackground(BLACK);

            fuelBarRect.height = fuel;
            DrawRectangleRounded(fuelBarBackground, radius, 8, DARKGRAY);
            DrawRectangleRounded(fuelBarRect, radius, 8, ORANGE);

            DrawRectangle(fuelRect.x, fuelRect.y, fuelRect.width, fuelRect.height, ORANGE);

            DrawText("o", px, py, 20, WHITE);
            DrawText(TextFormat("Score: %i", score), 700, 10, 20, WHITE);

            /* DrawText(TextFormat("x: %d", px), 700, 10, 20, WHITE);
            DrawText(TextFormat("y: %d", py), 700, 30, 20, WHITE);
            DrawText(TextFormat("playing: %d", isPlaying), 700, 50, 20, WHITE);
            DrawText(TextFormat("fuel: %f", fuel), 700, 70, 20, WHITE); useless debug messages */
            int endScore = score;
            if (isPlaying == false) {
                
                DrawRectangle(0,0,screenWidth,screenHeight,BLACK);

                const char* text = "You Lost!";
                const float textWidth = MeasureText(text, 50);
                const float textX = (screenWidth - textWidth) / 2.0f;
                const float textY = (screenHeight - 150.0f) / 2.0f;

                DrawText(text, textX, textY, 50, WHITE);

                const char* scoreText = TextFormat("Your score was: %i", endScore);
                const float scoreTextWidth = MeasureText(scoreText, 30);
                const float scoreTextX = (screenWidth - scoreTextWidth) / 2.0f;
                const float scoreTextY = textY + 70.0f;

                DrawText(scoreText, scoreTextX, scoreTextY, 30, WHITE);

                const char* replayText = "Press space to replay";
                const float replayTextWidth = MeasureText(replayText, 30);
                const float replayTextX = (screenWidth - replayTextWidth) / 2.0f;
                const float replayTextY = scoreTextY + 50.0f;

                DrawText(replayText, replayTextX, replayTextY, 30, WHITE);

                if (IsKeyPressed(KEY_SPACE)) {
                    fuel = 100.0;
                    g = 0.0;
                    px = screenWidth / 2;
                    py = screenHeight / 2;
                    isPlaying = true;
                    score = 0;
                }
            }

        EndDrawing();

        py -= g;
        g -= 0.17;

        if (px > screenWidth) {
            px = 0;
        }
        else if (px < 0) {
            px = screenWidth;
        }

        if (py <= 0) {
            py = 0;
        }

        if (py > screenHeight) {
            isPlaying = false;
        }

        if (!isFuelSpawned) {
            fuelRect.x = GetRandomValue(50, screenWidth - 20);
            fuelRect.y = GetRandomValue(20, screenHeight - 50);
            isFuelSpawned = true;
        }
    }
    CloseWindow();

    return 0;
}