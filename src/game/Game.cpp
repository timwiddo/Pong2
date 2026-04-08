#include "game/Game.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
constexpr int targetScore = 3;
constexpr int winCoins = 10;
constexpr int lossCoins = 3;
constexpr float playerSpeed = 520.0F;
constexpr float cpuSpeed = 460.0F;
constexpr float ballBaseSpeed = 680.0F;
constexpr float defaultPaddleHeight = 150.0F;
constexpr float paddleWidth = 28.0F;
constexpr float defaultBallRadius = 20.0F;
constexpr float cpuDeadZone = 10.0F;
constexpr float paddleMargin = 40.0F;

constexpr float minBallRadius = 8.0F;
constexpr float maxBallRadius = 34.0F;
constexpr float ballRadiusStep = 2.0F;
constexpr float minPaddleHeight = 80.0F;
constexpr float maxPaddleHeight = 230.0F;
constexpr float paddleHeightStep = 10.0F;

constexpr int mainMenuOptionCount = 3;
constexpr int pauseOptionCount = 2;
constexpr int settingsOptionCount = 3;
constexpr int shopCategoryCount = 3;
constexpr int skinCount = 4;

struct Skin {
    const char* name;
    Color color;
    int price;
};

constexpr std::array<Skin, skinCount> fieldSkins = {{
    {"Dark",    {18,  18,  24,  255},  0},
    {"Navy",    {10,  15,  50,  255}, 15},
    {"Forest",  {8,   35,  15,  255}, 15},
    {"Crimson", {42,  8,   10,  255}, 20},
}};

constexpr std::array<Skin, skinCount> paddleSkins = {{
    {"White",  {230, 230, 230, 255},  0},
    {"Gold",   {246, 212, 66,  255}, 20},
    {"Cyan",   {80,  220, 240, 255}, 25},
    {"Lime",   {120, 240, 100, 255}, 25},
}};

constexpr std::array<Skin, skinCount> ballSkins = {{
    {"Amber",  {235, 180, 80,  255},  0},
    {"Red",    {240, 80,  80,  255}, 20},
    {"Purple", {180, 80,  240, 255}, 25},
    {"Ice",    {130, 220, 255, 255}, 20},
}};

Color DeriveLineColor(Color bg) {
    return Color{
        static_cast<unsigned char>(std::min(static_cast<int>(bg.r) + 35, 255)),
        static_cast<unsigned char>(std::min(static_cast<int>(bg.g) + 35, 255)),
        static_cast<unsigned char>(std::min(static_cast<int>(bg.b) + 45, 255)),
        255
    };
}

std::filesystem::path ResolveCoinsSaveFilePath() {
    if (const char* localAppData = std::getenv("LOCALAPPDATA"); localAppData != nullptr && localAppData[0] != '\0') {
        return std::filesystem::path(localAppData) / "Pong2" / "coins.dat";
    }

    if (const char* userProfile = std::getenv("USERPROFILE"); userProfile != nullptr && userProfile[0] != '\0') {
        return std::filesystem::path(userProfile) / "Pong2" / "coins.dat";
    }

    // Last-resort fallback keeps the game functional in unusual environments.
    return std::filesystem::path("coins.dat");
}

Rectangle MainMenuButtonBounds(const int index, const int screenWidth, const int screenHeight) {
    constexpr float buttonWidth = 340.0F;
    constexpr float buttonHeight = 70.0F;
    constexpr float spacing = 18.0F;

    const float totalHeight = mainMenuOptionCount * buttonHeight + (mainMenuOptionCount - 1) * spacing;
    const float startY = screenHeight * 0.5F - totalHeight * 0.5F;
    const float x = screenWidth * 0.5F - buttonWidth * 0.5F;
    const float y = startY + index * (buttonHeight + spacing);
    return Rectangle{x, y, buttonWidth, buttonHeight};
}

Rectangle PauseButtonBounds(const int index, const int screenWidth, const int screenHeight) {
    constexpr float buttonWidth = 300.0F;
    constexpr float buttonHeight = 62.0F;
    constexpr float spacing = 14.0F;

    const float totalHeight = pauseOptionCount * buttonHeight + (pauseOptionCount - 1) * spacing;
    const float startY = screenHeight * 0.5F - totalHeight * 0.5F + 40.0F;
    const float x = screenWidth * 0.5F - buttonWidth * 0.5F;
    const float y = startY + index * (buttonHeight + spacing);
    return Rectangle{x, y, buttonWidth, buttonHeight};
}

Rectangle ShopTabBounds(const int index, const int screenWidth) {
    constexpr float tabWidth = 200.0F;
    constexpr float tabHeight = 56.0F;
    constexpr float spacing = 16.0F;

    const float totalWidth = shopCategoryCount * tabWidth + (shopCategoryCount - 1) * spacing;
    const float startX = screenWidth * 0.5F - totalWidth * 0.5F;
    return Rectangle{startX + index * (tabWidth + spacing), 210.0F, tabWidth, tabHeight};
}

Rectangle ShopItemBounds(const int index, const int screenWidth) {
    constexpr float itemWidth = 620.0F;
    constexpr float itemHeight = 74.0F;
    constexpr float spacing = 12.0F;
    constexpr float startY = 300.0F;

    return Rectangle{screenWidth * 0.5F - itemWidth * 0.5F, startY + index * (itemHeight + spacing), itemWidth, itemHeight};
}
}  // namespace

Game::Game(const int screenWidth, const int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      ballRadius_(defaultBallRadius),
      paddleHeight_(defaultPaddleHeight),
      playerPaddle_(Rectangle{paddleMargin, screenHeight * 0.5F - paddleHeight_ * 0.5F, paddleWidth, paddleHeight_}, playerSpeed),
      cpuPaddle_(Rectangle{screenWidth - (paddleMargin + paddleWidth), screenHeight * 0.5F - paddleHeight_ * 0.5F, paddleWidth, paddleHeight_}, cpuSpeed),
      pongBall_(Vector2{screenWidth * 0.5F, screenHeight * 0.5F}, ballRadius_, ballBaseSpeed),
      scoreBoard_(targetScore) {
    CenterPaddles();
    ServeFromCenter((GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
    LoadCoinsFromFile();
}

void Game::Run() {
    while (!WindowShouldClose()) {
        Update(GetFrameTime());

        BeginDrawing();
        Draw();
        EndDrawing();
    }

    SaveCoinsToFile();
}

void Game::Update(float deltaTime) {
    switch (gameState_) {
        case GameState::MainMenu:
            UpdateMainMenu();
            break;
        case GameState::Playing:
            if (IsKeyPressed(KEY_P)) {
                pauseSelection_ = 0;
                gameState_ = GameState::Paused;
                break;
            }
            UpdatePlaying(deltaTime);
            break;
        case GameState::Paused:
            UpdatePaused();
            break;
        case GameState::Shop:
            UpdateShop();
            break;
        case GameState::Settings:
            UpdateSettings();
            break;
        case GameState::GameOver:
            TryRestart();
            break;
    }
}

void Game::UpdateMainMenu() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        mainMenuSelection_ = (mainMenuSelection_ - 1 + mainMenuOptionCount) % mainMenuOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        mainMenuSelection_ = (mainMenuSelection_ + 1) % mainMenuOptionCount;
    }

    const Vector2 mousePosition = GetMousePosition();
    for (int index = 0; index < mainMenuOptionCount; ++index) {
        if (CheckCollisionPointRec(mousePosition, MainMenuButtonBounds(index, screenWidth_, screenHeight_))) {
            mainMenuSelection_ = index;
        }
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE) && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    if (mainMenuSelection_ == 0) {
        StartNewRound();
    } else if (mainMenuSelection_ == 1) {
        gameState_ = GameState::Shop;
    } else {
        settingsSelection_ = 0;
        gameState_ = GameState::Settings;
    }
}

void Game::UpdatePlaying(float deltaTime) {
    UpdatePlayerPaddle(deltaTime);
    UpdateCpuPaddle(deltaTime);

    pongBall_.Update(deltaTime);
    HandleBallCollisions();
    HandleScoring();

    if (scoreBoard_.HasWinner()) {
        coins_ += scoreBoard_.PlayerWon() ? winCoins : lossCoins;
        SaveCoinsToFile();
        gameState_ = GameState::GameOver;
    }
}

void Game::UpdatePaused() {
    if (IsKeyPressed(KEY_P)) {
        gameState_ = GameState::Playing;
        return;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        pauseSelection_ = (pauseSelection_ - 1 + pauseOptionCount) % pauseOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        pauseSelection_ = (pauseSelection_ + 1) % pauseOptionCount;
    }

    const Vector2 mousePosition = GetMousePosition();
    for (int index = 0; index < pauseOptionCount; ++index) {
        if (CheckCollisionPointRec(mousePosition, PauseButtonBounds(index, screenWidth_, screenHeight_))) {
            pauseSelection_ = index;
        }
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE) && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    if (pauseSelection_ == 0) {
        gameState_ = GameState::Playing;
    } else {
        ReturnToMainMenu();
    }
}

void Game::UpdateShop() {
    // Keyboard: switch category with A/D
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        shopCategory_ = (shopCategory_ - 1 + shopCategoryCount) % shopCategoryCount;
        shopItemSel_ = 0;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        shopCategory_ = (shopCategory_ + 1) % shopCategoryCount;
        shopItemSel_ = 0;
    }

    // Keyboard: navigate items with W/S
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        shopItemSel_ = (shopItemSel_ - 1 + skinCount) % skinCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        shopItemSel_ = (shopItemSel_ + 1) % skinCount;
    }

    // Mouse: hover over tabs to switch category
    const Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < shopCategoryCount; ++i) {
        if (CheckCollisionPointRec(mousePos, ShopTabBounds(i, screenWidth_))) {
            if (shopCategory_ != i) {
                shopCategory_ = i;
                shopItemSel_ = 0;
            }
        }
    }

    // Mouse: hover over items to highlight
    for (int i = 0; i < skinCount; ++i) {
        if (CheckCollisionPointRec(mousePos, ShopItemBounds(i, screenWidth_))) {
            shopItemSel_ = i;
        }
    }

    // Action: buy or equip via keyboard or item click
    bool actionTriggered = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (!actionTriggered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < skinCount; ++i) {
            if (CheckCollisionPointRec(mousePos, ShopItemBounds(i, screenWidth_))) {
                actionTriggered = true;
                break;
            }
        }
    }

    if (actionTriggered) {
        bool* ownedPtr = nullptr;
        int* activePtr = nullptr;
        const Skin* skinsPtr = nullptr;
        if (shopCategory_ == 0) {
            ownedPtr = fieldOwned_.data();
            activePtr = &activeFieldSkin_;
            skinsPtr = fieldSkins.data();
        } else if (shopCategory_ == 1) {
            ownedPtr = paddleOwned_.data();
            activePtr = &activePaddleSkin_;
            skinsPtr = paddleSkins.data();
        } else {
            ownedPtr = ballOwned_.data();
            activePtr = &activeBallSkin_;
            skinsPtr = ballSkins.data();
        }

        if (!ownedPtr[shopItemSel_]) {
            if (coins_ >= skinsPtr[shopItemSel_].price) {
                coins_ -= skinsPtr[shopItemSel_].price;
                ownedPtr[shopItemSel_] = true;
                *activePtr = shopItemSel_;
                SaveCoinsToFile();
            }
        } else {
            *activePtr = shopItemSel_;
            SaveCoinsToFile();
        }
    }

    // Back to main menu
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M)) {
        ReturnToMainMenu();
    }
}

void Game::UpdateSettings() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        settingsSelection_ = (settingsSelection_ - 1 + settingsOptionCount) % settingsOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        settingsSelection_ = (settingsSelection_ + 1) % settingsOptionCount;
    }

    float delta = 0.0F;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        delta = -1.0F;
    } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        delta = 1.0F;
    }

    if (delta != 0.0F) {
        if (settingsSelection_ == 0) {
            ballRadius_ = std::clamp(ballRadius_ + delta * ballRadiusStep, minBallRadius, maxBallRadius);
            pongBall_.SetRadius(ballRadius_);
        } else if (settingsSelection_ == 1) {
            paddleHeight_ = std::clamp(paddleHeight_ + delta * paddleHeightStep, minPaddleHeight, maxPaddleHeight);
            playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
            cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
            CenterPaddles();
        } else {
            ballRadius_ = defaultBallRadius;
            paddleHeight_ = defaultPaddleHeight;
            pongBall_.SetRadius(ballRadius_);
            playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
            cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
            CenterPaddles();
        }
    }

    if ((settingsSelection_ == 2) && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
        ballRadius_ = defaultBallRadius;
        paddleHeight_ = defaultPaddleHeight;
        pongBall_.SetRadius(ballRadius_);
        playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
        cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
        CenterPaddles();
    }

    if (IsKeyPressed(KEY_M)) {
        ReturnToMainMenu();
    }
}

void Game::UpdatePlaceholderScreen() {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ReturnToMainMenu();
    }
}

void Game::TryRestart() {
    if (IsKeyPressed(KEY_R)) {
        StartNewRound();
        return;
    }

    if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
        ReturnToMainMenu();
    }
}

void Game::StartNewRound() {
    scoreBoard_.Reset();
    gameState_ = GameState::Playing;
    pongBall_.SetRadius(ballRadius_);
    playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
    cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
    CenterPaddles();
    ServeFromCenter((GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
}

void Game::ReturnToMainMenu() {
    gameState_ = GameState::MainMenu;
    mainMenuSelection_ = 0;
    pauseSelection_ = 0;
}

void Game::Draw() const {
    ClearBackground(fieldSkins[activeFieldSkin_].color);

    switch (gameState_) {
        case GameState::MainMenu:
            DrawMainMenu();
            break;
        case GameState::Shop:
            DrawShop();
            break;
        case GameState::Settings:
            DrawSettings();
            break;
        case GameState::Playing:
            DrawGameplay();
            break;
        case GameState::Paused:
            DrawGameplay();
            DrawPausedOverlay();
            break;
        case GameState::GameOver:
            DrawGameplay();
            DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.45F));
            DrawText(scoreBoard_.PlayerWon() ? "You Win!" : "Computer Wins!", screenWidth_ / 2 - 150, screenHeight_ / 2 - 50, 48, RAYWHITE);
            DrawText("Press R to play again", screenWidth_ / 2 - 135, screenHeight_ / 2 + 14, 24, Color{220, 220, 220, 255});
            DrawText("Press M or Esc for main menu", screenWidth_ / 2 - 168, screenHeight_ / 2 + 48, 22, Color{200, 200, 210, 255});
            break;
    }

    DrawCoinsHud();
}

void Game::DrawMainMenu() const {
    DrawText("PONG", screenWidth_ / 2 - 100, 130, 84, RAYWHITE);
    DrawText("Choose an option", screenWidth_ / 2 - 120, 230, 30, Color{185, 185, 200, 255});

    constexpr std::array<const char*, mainMenuOptionCount> options = {"Play", "Shop", "Settings"};

    for (int index = 0; index < mainMenuOptionCount; ++index) {
        const Rectangle bounds = MainMenuButtonBounds(index, screenWidth_, screenHeight_);
        const bool selected = index == mainMenuSelection_;

        DrawRectangleRec(bounds, selected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
        DrawRectangleLinesEx(bounds, 2.0F, selected ? RAYWHITE : Color{90, 90, 110, 255});

        DrawText(options[index], static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 18, 34,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});
    }

    DrawText("Use mouse or W/S (Up/Down), Enter to choose", screenWidth_ / 2 - 255, screenHeight_ - 90, 22, Color{165, 165, 180, 255});
}

void Game::DrawGameplay() const {
    DrawLine(screenWidth_ / 2, 0, screenWidth_ / 2, screenHeight_, DeriveLineColor(fieldSkins[activeFieldSkin_].color));
    playerPaddle_.Draw(paddleSkins[activePaddleSkin_].color);
    cpuPaddle_.Draw(paddleSkins[activePaddleSkin_].color);
    pongBall_.Draw(ballSkins[activeBallSkin_].color);
    scoreBoard_.Draw(screenWidth_);

    DrawText("W/S or UP/DOWN to move", 28, screenHeight_ - 72, 20, Color{160, 160, 170, 255});
    DrawText("P to pause", 28, screenHeight_ - 42, 20, Color{160, 160, 170, 255});
}

void Game::DrawPausedOverlay() const {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.55F));
    DrawText("Paused", screenWidth_ / 2 - 90, screenHeight_ / 2 - 110, 58, RAYWHITE);

    constexpr std::array<const char*, pauseOptionCount> options = {"Resume", "Main Menu"};
    for (int index = 0; index < pauseOptionCount; ++index) {
        const Rectangle bounds = PauseButtonBounds(index, screenWidth_, screenHeight_);
        const bool selected = index == pauseSelection_;

        DrawRectangleRec(bounds, selected ? Color{70, 110, 200, 255} : Color{48, 48, 62, 255});
        DrawRectangleLinesEx(bounds, 2.0F, selected ? RAYWHITE : Color{95, 95, 115, 255});
        DrawText(options[index], static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 16, 32,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});
    }

    DrawText("P resumes", screenWidth_ / 2 - 62, screenHeight_ / 2 + 192, 20, Color{180, 180, 190, 255});
}

void Game::DrawShop() const {
    DrawText("Shop", screenWidth_ / 2 - 70, 110, 72, RAYWHITE);

    // Category tabs
    constexpr std::array<const char*, shopCategoryCount> categoryNames = {"Field", "Paddles", "Ball"};
    for (int i = 0; i < shopCategoryCount; ++i) {
        const Rectangle tab = ShopTabBounds(i, screenWidth_);
        const bool selected = i == shopCategory_;
        DrawRectangleRec(tab, selected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
        DrawRectangleLinesEx(tab, 2.0F, selected ? RAYWHITE : Color{90, 90, 110, 255});
        DrawText(categoryNames[i], static_cast<int>(tab.x) + 20, static_cast<int>(tab.y) + 14, 28,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});
    }

    // Determine skins and state for the active category
    const Skin* skins = nullptr;
    const bool* owned = nullptr;
    int activeIndex = 0;
    if (shopCategory_ == 0) {
        skins = fieldSkins.data();
        owned = fieldOwned_.data();
        activeIndex = activeFieldSkin_;
    } else if (shopCategory_ == 1) {
        skins = paddleSkins.data();
        owned = paddleOwned_.data();
        activeIndex = activePaddleSkin_;
    } else {
        skins = ballSkins.data();
        owned = ballOwned_.data();
        activeIndex = activeBallSkin_;
    }

    // Draw item rows
    for (int i = 0; i < skinCount; ++i) {
        const Rectangle row = ShopItemBounds(i, screenWidth_);
        const bool selected = i == shopItemSel_;
        DrawRectangleRec(row, selected ? Color{60, 100, 200, 255} : Color{45, 45, 60, 255});
        DrawRectangleLinesEx(row, 2.0F, selected ? RAYWHITE : Color{90, 90, 110, 255});

        // Color preview swatch
        DrawRectangle(static_cast<int>(row.x) + 14, static_cast<int>(row.y) + 15, 44, 44, skins[i].color);
        DrawRectangleLines(static_cast<int>(row.x) + 14, static_cast<int>(row.y) + 15, 44, 44,
                           selected ? RAYWHITE : Color{150, 150, 160, 255});

        // Skin name
        DrawText(skins[i].name, static_cast<int>(row.x) + 72, static_cast<int>(row.y) + 21, 30,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});

        // Status label
        if (i == activeIndex) {
            DrawText("ACTIVE", static_cast<int>(row.x) + 450, static_cast<int>(row.y) + 23, 26, Color{100, 220, 100, 255});
        } else if (owned[i]) {
            DrawText("OWNED", static_cast<int>(row.x) + 450, static_cast<int>(row.y) + 23, 26, Color{160, 160, 190, 255});
        } else {
            DrawText(TextFormat("%d coins", skins[i].price), static_cast<int>(row.x) + 420, static_cast<int>(row.y) + 23, 26,
                     coins_ >= skins[i].price ? Color{246, 212, 66, 255} : Color{180, 100, 100, 255});
        }
    }

    DrawText("A/D - category   W/S - select   Enter - buy/equip   Esc/M - back",
             screenWidth_ / 2 - 380, screenHeight_ - 90, 22, Color{155, 155, 172, 255});
}

void Game::DrawSettings() const {
    DrawText("Settings", screenWidth_ / 2 - 130, 105, 72, RAYWHITE);
    DrawText("Use W/S to select and A/D to change", screenWidth_ / 2 - 215, 200, 28, Color{185, 185, 205, 255});

    const bool ballSelected = settingsSelection_ == 0;
    const bool paddleSelected = settingsSelection_ == 1;
    const bool resetSelected = settingsSelection_ == 2;

    const Rectangle ballRow{screenWidth_ * 0.5F - 250.0F, 260.0F, 500.0F, 76.0F};
    DrawRectangleRec(ballRow, ballSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(ballRow, 2.0F, ballSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText(TextFormat("Ball Size: %.0f", ballRadius_), static_cast<int>(ballRow.x) + 20, static_cast<int>(ballRow.y) + 22, 34,
             ballSelected ? RAYWHITE : Color{220, 220, 230, 255});

    const Rectangle paddleRow{screenWidth_ * 0.5F - 250.0F, 358.0F, 500.0F, 76.0F};
    DrawRectangleRec(paddleRow, paddleSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(paddleRow, 2.0F, paddleSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText(TextFormat("Paddle Size: %.0f", paddleHeight_), static_cast<int>(paddleRow.x) + 20, static_cast<int>(paddleRow.y) + 22, 34,
             paddleSelected ? RAYWHITE : Color{220, 220, 230, 255});

    const Rectangle resetRow{screenWidth_ * 0.5F - 250.0F, 456.0F, 500.0F, 76.0F};
    DrawRectangleRec(resetRow, resetSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(resetRow, 2.0F, resetSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText("Reset to Default", static_cast<int>(resetRow.x) + 20, static_cast<int>(resetRow.y) + 22, 34,
             resetSelected ? RAYWHITE : Color{220, 220, 230, 255});

    DrawText(TextFormat("Ball range: %.0f - %.0f", minBallRadius, maxBallRadius), screenWidth_ / 2 - 145, 552, 24,
             Color{170, 170, 188, 255});
    DrawText(TextFormat("Paddle range: %.0f - %.0f", minPaddleHeight, maxPaddleHeight), screenWidth_ / 2 - 165, 585, 24,
             Color{170, 170, 188, 255});
    DrawText("Enter/Space on reset; M to go back", screenWidth_ / 2 - 205, screenHeight_ - 95, 24, Color{155, 155, 172, 255});
}

void Game::DrawPlaceholderScreen(const char* title) const {
    DrawText(title, screenWidth_ / 2 - 120, 220, 74, RAYWHITE);
    DrawText("Coming soon", screenWidth_ / 2 - 110, screenHeight_ / 2 - 12, 44, Color{190, 190, 210, 255});
    DrawText("Press Esc, Enter, or click to return", screenWidth_ / 2 - 210, screenHeight_ - 100, 26,
             Color{155, 155, 172, 255});
}

void Game::DrawCoinsHud() const {
    const int circleX = screenWidth_ - 135;
    const int circleY = 42;
    DrawCircle(circleX, circleY, 16.0F, Color{246, 212, 66, 255});
    DrawCircleLines(circleX, circleY, 16.0F, Color{255, 236, 120, 255});
    DrawText(TextFormat("%d", coins_), screenWidth_ - 105, 25, 34, Color{246, 212, 66, 255});
}

void Game::LoadCoinsFromFile() {
    const std::filesystem::path savePath = ResolveCoinsSaveFilePath();
    std::ifstream input(savePath);
    int loadedCoins = 0;
    if (!(input >> loadedCoins) || loadedCoins < 0) {
        coins_ = 0;
        return;
    }
    coins_ = loadedCoins;

    // Active skin indices (optional — missing in older save files)
    int af = 0, ap = 0, ab = 0;
    if (input >> af >> ap >> ab) {
        activeFieldSkin_  = std::clamp(af, 0, skinCount - 1);
        activePaddleSkin_ = std::clamp(ap, 0, skinCount - 1);
        activeBallSkin_   = std::clamp(ab, 0, skinCount - 1);
    }

    // Ownership arrays (optional)
    auto loadOwned = [&](std::array<bool, skinCount>& arr) {
        arr.fill(false);
        for (int i = 0; i < skinCount; ++i) {
            int v = 0;
            if (input >> v) {
                arr[i] = (v != 0);
            }
        }
        arr[0] = true; // first skin is always owned
    };
    loadOwned(fieldOwned_);
    loadOwned(paddleOwned_);
    loadOwned(ballOwned_);

    // Validate active indices point to owned skins
    if (!fieldOwned_[activeFieldSkin_])   activeFieldSkin_  = 0;
    if (!paddleOwned_[activePaddleSkin_]) activePaddleSkin_ = 0;
    if (!ballOwned_[activeBallSkin_])     activeBallSkin_   = 0;
}

void Game::SaveCoinsToFile() const {
    const std::filesystem::path savePath = ResolveCoinsSaveFilePath();
    if (savePath.has_parent_path()) {
        std::error_code errorCode;
        std::filesystem::create_directories(savePath.parent_path(), errorCode);
    }

    std::ofstream output(savePath, std::ios::trunc);
    if (!output) {
        return;
    }

    output << coins_ << "\n";
    output << activeFieldSkin_ << " " << activePaddleSkin_ << " " << activeBallSkin_ << "\n";

    auto saveOwned = [&](const std::array<bool, skinCount>& arr) {
        for (int i = 0; i < skinCount; ++i) {
            output << (arr[i] ? 1 : 0);
            if (i < skinCount - 1) { output << " "; }
        }
        output << "\n";
    };
    saveOwned(fieldOwned_);
    saveOwned(paddleOwned_);
    saveOwned(ballOwned_);
}

Vector2 Game::ScreenCenter() const {
    return Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F};
}

void Game::ServeFromCenter(float horizontalDirection) {
    pongBall_.Reset(ScreenCenter(), horizontalDirection);
}

void Game::CenterPaddles() {
    const float centeredPaddleY = screenHeight_ * 0.5F - paddleHeight_ * 0.5F;
    playerPaddle_.SetY(centeredPaddleY);
    cpuPaddle_.SetY(centeredPaddleY);
}

void Game::UpdatePlayerPaddle(float deltaTime) {
    float playerMoveDirection = 0.0F;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        playerMoveDirection -= 1.0F;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        playerMoveDirection += 1.0F;
    }
    playerPaddle_.Move(playerMoveDirection, deltaTime, screenHeight_);
}

void Game::UpdateCpuPaddle(float deltaTime) {
    const float cpuTrackingOffset = pongBall_.Y() - cpuPaddle_.CenterY();
    if (std::fabs(cpuTrackingOffset) > cpuDeadZone) {
        cpuPaddle_.Move((cpuTrackingOffset > 0.0F) ? 1.0F : -1.0F, deltaTime, screenHeight_);
    }
}

void Game::HandleBallCollisions() {
    const Rectangle ballBounds = pongBall_.Bounds();

    if (ballBounds.y <= 0.0F) {
        pongBall_.BounceFromTop();
    } else if (ballBounds.y + ballBounds.height >= screenHeight_) {
        pongBall_.BounceFromBottom(static_cast<float>(screenHeight_));
    }

    if (CheckCollisionRecs(ballBounds, playerPaddle_.Bounds()) && pongBall_.VelocityX() < 0.0F) {
        pongBall_.BounceFromPaddle(playerPaddle_, true);
    } else if (CheckCollisionRecs(ballBounds, cpuPaddle_.Bounds()) && pongBall_.VelocityX() > 0.0F) {
        pongBall_.BounceFromPaddle(cpuPaddle_, false);
    }
}

void Game::HandleScoring() {
    if (pongBall_.IsOutLeft()) {
        scoreBoard_.CpuScored();
        ServeFromCenter(1.0F);
    } else if (pongBall_.IsOutRight(static_cast<float>(screenWidth_))) {
        scoreBoard_.PlayerScored();
        ServeFromCenter(-1.0F);
    }
}
