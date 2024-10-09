#include "raylib.h"
#include "raymath.h"

#define MAX_ASTEROIDS 10
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define PLAYER_SIZE 20
#define BASE_FUEL 100.0
#define FUEL_PICKUP_VALUE 15
#define UPGRADE_FUEL_BONUS 10

typedef enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
} GameState;

typedef struct {
    Rectangle rect;
    Vector2 speed;
    float rotation;
    Rectangle hitbox;
} Asteroid;

typedef struct {
    Vector2 position;
    Rectangle rect;
    float gravity;
    bool isJetpacking;
} Player;

typedef struct {
    Rectangle rect;
    bool isSpawned;
    Texture2D texture;
} Pickup;

typedef struct {
    double current;
    int maximum;
    Rectangle barRect;
    Rectangle backgroundRect;
} FuelSystem;

typedef struct {
    int score;
    Sound rocketSound;
    Sound hitSound;
    Sound collectSound;
    bool isSoundPlaying;
    Texture2D asteroidTexture;
} GameResources;

void InitGame(Player* player, FuelSystem* fuel, Pickup* fuelPickup, Pickup* upgrade,
             Asteroid* asteroids, GameResources* resources);
void UpdatePlayer(Player* player, FuelSystem* fuel, GameResources* resources);
void UpdateAsteroids(Asteroid* asteroids, int* numAsteroids, Player* player,
                    FuelSystem* fuel, GameResources* resources);
void UpdatePickups(Player* player, Pickup* fuelPickup, Pickup* upgrade,
                  FuelSystem* fuel, GameResources* resources);
void DrawGame(Player player, Asteroid* asteroids, int numAsteroids,
             Pickup fuelPickup, Pickup upgrade, FuelSystem fuel, GameResources resources);
void DrawUI(GameState state, FuelSystem fuel, GameResources resources);
void SpawnAsteroid(Asteroid* asteroid, int score);

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jetpack Fella");
    InitAudioDevice();
    SetTargetFPS(60);

    Player player = {0};
    FuelSystem fuel = {0};
    Pickup fuelPickup = {0};
    Pickup upgrade = {0};
    Asteroid asteroids[MAX_ASTEROIDS] = {0};
    GameResources resources = {0};
    GameState gameState = MENU;
    int numAsteroids = 0;

    InitGame(&player, &fuel, &fuelPickup, &upgrade, asteroids, &resources);

    while (!WindowShouldClose()) {
        switch (gameState) {
            case MENU:
                if (IsKeyPressed(KEY_SPACE)) {
                    gameState = PLAYING;
                }
                break;

            case PLAYING:
                UpdatePlayer(&player, &fuel, &resources);
                UpdateAsteroids(asteroids, &numAsteroids, &player, &fuel, &resources);
                UpdatePickups(&player, &fuelPickup, &upgrade, &fuel, &resources);

                if (player.position.y > SCREEN_HEIGHT || fuel.current <= 0) {
                    gameState = GAME_OVER;
                }
                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_SPACE)) {
                    InitGame(&player, &fuel, &fuelPickup, &upgrade, asteroids, &resources);
                    gameState = PLAYING;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawGame(player, asteroids, numAsteroids, fuelPickup, upgrade, fuel, resources);
        DrawUI(gameState, fuel, resources);

        EndDrawing();
    }

    UnloadTexture(resources.asteroidTexture);
    UnloadTexture(fuelPickup.texture);
    UnloadSound(resources.rocketSound);
    UnloadSound(resources.hitSound);
    UnloadSound(resources.collectSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void InitGame(Player* player, FuelSystem* fuel, Pickup* fuelPickup, Pickup* upgrade,
             Asteroid* asteroids, GameResources* resources) {
    player->position = (Vector2){ SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };
    player->rect = (Rectangle){ player->position.x, player->position.y, PLAYER_SIZE, PLAYER_SIZE };
    player->gravity = 0;
    player->isJetpacking = false;

    fuel->current = BASE_FUEL;
    fuel->maximum = BASE_FUEL;
    fuel->barRect = (Rectangle){ 15, 15, 20, BASE_FUEL };
    fuel->backgroundRect = (Rectangle){ 10, 10, 30, BASE_FUEL + 10 };

    fuelPickup->rect = (Rectangle){ GetRandomValue(0, SCREEN_WIDTH - 20),
                                   GetRandomValue(0, SCREEN_HEIGHT - 20), 20, 20 };
    fuelPickup->isSpawned = true;

    upgrade->rect = (Rectangle){ -100, -100, 20, 20 };
    upgrade->isSpawned = false;

    if (resources->asteroidTexture.id == 0) {
        Image asteroidImg = LoadImage("image/asteroid.png");
        resources->asteroidTexture = LoadTextureFromImage(asteroidImg);
        UnloadImage(asteroidImg);

        Image jerrycanImg = LoadImage("image/jerrycan.png");
        ImageResize(&jerrycanImg, 25, 25);
        fuelPickup->texture = LoadTextureFromImage(jerrycanImg);
        UnloadImage(jerrycanImg);

        resources->rocketSound = LoadSound("sound/rocket.mp3");
        resources->hitSound = LoadSound("sound/hit.mp3");
        resources->collectSound = LoadSound("sound/collect.mp3");
    }

    resources->score = 0;
    resources->isSoundPlaying = false;

    memset(asteroids, 0, sizeof(Asteroid) * MAX_ASTEROIDS);
}

void UpdatePlayer(Player* player, FuelSystem* fuel, GameResources* resources) {
    float moveSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 4.0f : 2.0f;

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->position.x += moveSpeed;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->position.x -= moveSpeed;
    }

    if (IsKeyDown(KEY_SPACE) && fuel->current > 0) {
        player->gravity = IsKeyDown(KEY_S) ? -4.0f : 4.0f;

        if (!resources->isSoundPlaying) {
            PlaySound(resources->rocketSound);
            resources->isSoundPlaying = true;
        }

        fuel->current -= 0.5;
    } else {
        if (resources->isSoundPlaying) {
            StopSound(resources->rocketSound);
            resources->isSoundPlaying = false;
        }
    }

    player->position.y -= player->gravity;
    player->gravity -= 0.17f;

    if (player->position.x > SCREEN_WIDTH) player->position.x = 0;
    else if (player->position.x < 0) player->position.x = SCREEN_WIDTH;
    if (player->position.y < 0) player->position.y = 0;

    player->rect.x = player->position.x;
    player->rect.y = player->position.y;
}

void UpdateAsteroids(Asteroid* asteroids, int* numAsteroids, Player* player,
                    FuelSystem* fuel, GameResources* resources) {
    if (*numAsteroids < MAX_ASTEROIDS && GetRandomValue(0, 100) < 10 && resources->score >= 3) {
        SpawnAsteroid(&asteroids[*numAsteroids], resources->score);
        (*numAsteroids)++;
    }

    for (int i = 0; i < *numAsteroids; i++) {
        asteroids[i].rect.y += asteroids[i].speed.y;

        asteroids[i].hitbox = (Rectangle){
            asteroids[i].rect.x + asteroids[i].rect.width * 0.25f,
            asteroids[i].rect.y + asteroids[i].rect.height * 0.25f,
            asteroids[i].rect.width * 0.5f,
            asteroids[i].rect.height * 0.5f
        };

        if (CheckCollisionRecs(player->rect, asteroids[i].hitbox)) {
            PlaySound(resources->hitSound);
            float damage = asteroids[i].rect.width/3;
            fuel->current = (fuel->current - damage > 0) ? fuel->current - damage : 0;
            SpawnAsteroid(&asteroids[i], resources->score);
        }

        if (asteroids[i].rect.y > SCREEN_HEIGHT) {
            SpawnAsteroid(&asteroids[i], resources->score);
        }
    }
}

void SpawnAsteroid(Asteroid* asteroid, int score) {
    asteroid->rect.x = GetRandomValue(0, SCREEN_WIDTH - 50);
    asteroid->rect.y = 0;
    asteroid->rect.width = GetRandomValue(15, score * 3);
    asteroid->rect.height = asteroid->rect.width;
    asteroid->speed.x = 0;
    asteroid->speed.y = 80.0f / asteroid->rect.width;
    asteroid->rotation = GetRandomValue(0, 360);
}

void UpdatePickups(Player* player, Pickup* fuelPickup, Pickup* upgrade,
                  FuelSystem* fuel, GameResources* resources) {
    if (CheckCollisionRecs(player->rect, fuelPickup->rect)) {
        PlaySound(resources->collectSound);
        fuel->current = fmin(fuel->current + FUEL_PICKUP_VALUE, fuel->maximum);
        fuelPickup->isSpawned = false;
        resources->score++;
    }

    if (CheckCollisionRecs(player->rect, upgrade->rect) && upgrade->isSpawned) {
        fuel->maximum += UPGRADE_FUEL_BONUS;
        fuel->current += 5;
        resources->score++;
        upgrade->isSpawned = false;
    }

    if (!fuelPickup->isSpawned) {
        fuelPickup->rect.x = GetRandomValue(50, SCREEN_WIDTH - 20);
        fuelPickup->rect.y = GetRandomValue(20, SCREEN_HEIGHT - 50);
        fuelPickup->isSpawned = true;
    }

    if (resources->score > 0 && resources->score % 10 == 0 && !upgrade->isSpawned) {
        upgrade->rect.x = GetRandomValue(0, SCREEN_WIDTH - 20);
        upgrade->rect.y = GetRandomValue(0, SCREEN_HEIGHT - 20);
        upgrade->isSpawned = true;
    }
}

void DrawGame(Player player, Asteroid* asteroids, int numAsteroids,
             Pickup fuelPickup, Pickup upgrade, FuelSystem fuel, GameResources resources) {
    fuel.barRect.height = fuel.current;
    fuel.backgroundRect.height = fuel.maximum + 10;
    DrawRectangleRounded(fuel.backgroundRect, 0.1f, 8, DARKGRAY);
    DrawRectangleRounded(fuel.barRect, 0.1f, 8, ORANGE);

    DrawTexture(fuelPickup.texture, fuelPickup.rect.x, fuelPickup.rect.y, ORANGE);
    if (upgrade.isSpawned) {
        DrawRectangleRounded(upgrade.rect, 8, 0, GREEN);
    }

    DrawText("o", player.position.x, player.position.y, 20, WHITE);

    Rectangle sourceRect = { 0, 0, resources.asteroidTexture.width, resources.asteroidTexture.height };
    for (int i = 0; i < numAsteroids; i++) {
        DrawTexturePro(resources.asteroidTexture, sourceRect, asteroids[i].rect,
                      (Vector2){0, 0}, asteroids[i].rotation, WHITE);
    }

    DrawText(TextFormat("Score: %i", resources.score), 700, 10, 20, WHITE);
    DrawText(TextFormat("FPS: %i", GetFPS()), SCREEN_WIDTH-100, 30, 20, WHITE);
}

void DrawUI(GameState state, FuelSystem fuel, GameResources resources) {
    switch (state) {
        case MENU: {
            const char* menuText = "jetpack fella";
            const float menuTextWidth = MeasureText(menuText, 50);
            const float menuTextX = (SCREEN_WIDTH - menuTextWidth) / 2.0f;
            const float menuTextY = (SCREEN_HEIGHT - 150.0f) / 2.0f;

            DrawText(menuText, menuTextX, menuTextY, 50, WHITE);
            DrawText("space to play",
                    (SCREEN_WIDTH - MeasureText("space to play", 30)) / 2.0f,
                    menuTextY + 75.0f, 30, WHITE);
            break;
        }
        case GAME_OVER: {
            const char* gameOverText = "You Lost!";
            const float textWidth = MeasureText(gameOverText, 50);
            const float textX = (SCREEN_WIDTH - textWidth) / 2.0f;
            const float textY = (SCREEN_HEIGHT - 150.0f) / 2.0f;

            DrawText(gameOverText, textX, textY, 50, WHITE);
            DrawText(TextFormat("Your score was: %i", resources.score),
                    (SCREEN_WIDTH - MeasureText(TextFormat("Your score was: %i", resources.score), 30)) / 2.0f,
                    textY + 70.0f, 30, WHITE);
            DrawText("Press space to replay",
                    (SCREEN_WIDTH - MeasureText("Press space to replay", 30)) / 2.0f,
                    textY + 120.0f, 30, WHITE);
            break;
        }
        default:
            break;
    }
}
