#include <raylib.h>
#include <cmath>
#include <string>

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

enum GameState { MENU, GAME, PAUSE, GAME_OVER };

struct Player {
    Vector2 position;
    int health;
    float speed;
    Texture2D texture;
    float attackCooldown;
    float attackTimer;
};

struct Monster {
    Vector2 position;
    int health;
    float speed;
    Texture2D texture;
    bool isAlive;
    float attackCooldown;
};

struct Bonus {
    Vector2 position;
    bool active;
    Texture2D texture;
};

void ResetMonster(Monster& monster, Texture2D texture) {
    monster.health = 50;
    monster.isAlive = true;
    monster.attackCooldown = 0.0f;
    monster.position = { 1000.0f, (float)(100 + GetRandomValue(0, 520)) };
}


void SpawnHealthPickup(Bonus& bonus) {
    if (GetRandomValue(0, 100) < 50) {
        bonus.active = true;
        bonus.position = { (float)GetRandomValue(100, 1180), (float)GetRandomValue(100, 620) };
    }
}

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Battle with Monster");
    SetTargetFPS(60);
    
    InitAudioDevice();
    
    SetRandomSeed((unsigned int)GetTime());
    
    Texture2D backgroundTexture = LoadTexture("resources/background.png");
    Texture2D playerTexture = LoadTexture("resources/player.png");
    Texture2D monsterTexture = LoadTexture("resources/monster.png");
    Texture2D healthTexture = LoadTexture("resources/health.png");
    
    
    Sound attackSound = LoadSound("resources/attack.wav");
    Music backgroundMusic = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(backgroundMusic);
    
    
    GameState currentState = MENU;
    int highScore = 0;
    
    
    Player player = {
        { 200, 360 },
        100,
        200.0f,
        playerTexture,
        0.0f,
        0.0f
    };
    
    
    Monster monster = {
        { 1000, 360 },
        50,
        100.0f,
        monsterTexture,
        true,
        0.0f
    };
    
    
    Bonus healthPickup = {
        { 0, 0 },
        false,
        healthTexture
    };
    
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(backgroundMusic);
        
        float deltaTime = GetFrameTime();
        
        switch (currentState) {
            case MENU: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentState = GAME;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    CloseAudioDevice();
                    CloseWindow();
                    return 0;
                }
                break;
            }
            case GAME: {
                if (IsKeyPressed(KEY_P)) {
                    currentState = PAUSE;
                }
                
                
                if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed * deltaTime;
                if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed * deltaTime;
                if (IsKeyDown(KEY_UP)) player.position.y -= player.speed * deltaTime;
                if (IsKeyDown(KEY_DOWN)) player.position.y += player.speed * deltaTime;
                
                
                player.position.x = Clamp(player.position.x, 0, screenWidth - player.texture.width);
                player.position.y = Clamp(player.position.y, 0, screenHeight - player.texture.height);
                
                
                if (player.attackCooldown > 0) {
                    player.attackCooldown -= deltaTime;
                }
                if (player.attackTimer > 0) {
                    player.attackTimer -= deltaTime;
                }
                if (monster.attackCooldown > 0) {
                    monster.attackCooldown -= deltaTime;
                }
                
                
                Rectangle playerRec = { player.position.x, player.position.y, (float)player.texture.width, (float)player.texture.height };
                
                
                if (monster.isAlive) {
                    Vector2 direction = { player.position.x - monster.position.x, player.position.y - monster.position.y };
                    float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
                    if (distance > 0) {
                        direction.x /= distance;
                        direction.y /= distance;
                        monster.position.x += direction.x * monster.speed * deltaTime;
                        monster.position.y += direction.y * monster.speed * deltaTime;
                    }
                    
                    
                    Rectangle monsterRec = { monster.position.x, monster.position.y, (float)monster.texture.width, (float)monster.texture.height };
                    
                    if (CheckCollisionRecs(playerRec, monsterRec)) {
                        
                        if (IsKeyPressed(KEY_SPACE) && player.attackCooldown <= 0.0f) {
                            monster.health -= 10;
                            player.attackCooldown = 0.5f;
                            player.attackTimer = 0.2f;
                            PlaySound(attackSound);
                        }
                        
                        
                        if (monster.attackCooldown <= 0.0f) {
                            player.health -= 5;
                            monster.attackCooldown = 0.5f;
                        }
                    }
                    
                    
                    if (monster.health <= 0) {
                        monster.isAlive = false;
                        highScore += 100;
                        SpawnHealthPickup(healthPickup);
                        ResetMonster(monster, monsterTexture);
                    }
                }
                
                
                if (healthPickup.active) {
                    Rectangle healthRec = { healthPickup.position.x, healthPickup.position.y, (float)healthPickup.texture.width, (float)healthPickup.texture.height };
                    if (CheckCollisionRecs(playerRec, healthRec)) {
                        player.health = Clamp(player.health + 20, 0, 100);
                        healthPickup.active = false;
                    }
                }
                
                
                if (player.health <= 0) {
                    currentState = GAME_OVER;
                }
                break;
            }
            case PAUSE: {
                if (IsKeyPressed(KEY_P)) {
                    currentState = GAME;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentState = MENU;
                }
                break;
            }
            case GAME_OVER: {
                if (IsKeyPressed(KEY_ENTER)) {
                    player.health = 100;
                    player.position = { 200, 360 };
                    monster.health = 50;
                    monster.position = { 1000, 360 };
                    monster.isAlive = true;
                    monster.attackCooldown = 0.0f;
                    healthPickup.active = false;
                    currentState = MENU;
                }
                break;
            }
        }
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        switch (currentState) {
            case MENU: {
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawText("Battle with Monster", screenWidth/2 - MeasureText("Battle with Monster", 40)/2, 200, 40, BLACK);
                DrawText("Press ENTER to Start", screenWidth/2 - MeasureText("Press ENTER to Start", 30)/2, 300, 30, BLACK);
                DrawText("Press ESC to Exit", screenWidth/2 - MeasureText("Press ESC to Exit", 30)/2, 350, 30, BLACK);
                break;
            }
            case GAME: {
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                
                Color playerColor = (player.attackTimer > 0) ? RED : WHITE;
                DrawTexture(player.texture, (int)player.position.x, (int)player.position.y, playerColor);
                
                if (monster.isAlive) {
                    DrawTexture(monster.texture, (int)monster.position.x, (int)monster.position.y, WHITE);
                }
                
                if (healthPickup.active) {
                    DrawTexture(healthPickup.texture, (int)healthPickup.position.x, (int)healthPickup.position.y, WHITE);
                }
                
                DrawText(TextFormat("Player Health: %d", player.health), 20, 20, 20, RED);
                DrawText(TextFormat("Monster Health: %d", monster.health), 20, 50, 20, GREEN);
                DrawText(TextFormat("Score: %d", highScore), 20, 80, 20, BLACK);
                break;
            }
            case PAUSE: {
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawTexture(player.texture, (int)player.position.x, (int)player.position.y, WHITE);
                if (monster.isAlive) {
                    DrawTexture(monster.texture, (int)monster.position.x, (int)monster.position.y, WHITE);
                }
                if (healthPickup.active) {
                    DrawTexture(healthPickup.texture, (int)healthPickup.position.x, (int)healthPickup.position.y, WHITE);
                }
                DrawText("PAUSED", screenWidth/2 - MeasureText("PAUSED", 50)/2, screenHeight/2, 50, BLACK);
                DrawText("Press P to Resume", screenWidth/2 - MeasureText("Press P to Resume", 30)/2, screenHeight/2 + 60, 30, BLACK);
                DrawText("Press ESC to Menu", screenWidth/2 - MeasureText("Press ESC to Menu", 30)/2, screenHeight/2 + 100, 30, BLACK);
                break;
            }
            case GAME_OVER: {
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 50)/2, screenHeight/2, 50, RED);
                DrawText(TextFormat("Final Score: %d", highScore), screenWidth/2 - MeasureText(TextFormat("Final Score: %d", highScore), 30)/2, screenHeight/2 + 60, 30, BLACK);
                DrawText("Press ENTER to Restart", screenWidth/2 - MeasureText("Press ENTER to Restart", 30)/2, screenHeight/2 + 100, 30, BLACK);
                break;
            }
        }
        
        EndDrawing();
    }
    
    UnloadTexture(backgroundTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(monsterTexture);
    UnloadTexture(healthTexture);
    UnloadSound(attackSound);
    UnloadMusicStream(backgroundMusic);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}