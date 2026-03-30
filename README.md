# Pong2 (Raylib)

Simple Pong game in C++ using raylib.

## Project Structure

- `main.cpp` - Application bootstrap (window setup and game start)
- `include/game/Game.h` and `src/game/Game.cpp` - Main game loop and orchestration
- `include/entities/Paddle.h` and `src/entities/Paddle.cpp` - Paddle behavior
- `include/entities/Ball.h` and `src/entities/Ball.cpp` - Ball movement and collisions
- `include/ui/ScoreBoard.h` and `src/ui/ScoreBoard.cpp` - Score handling and drawing

## Features

- Home screen with `Play`, `Shop`, and `Settings`
- `Play` starts a fresh Pong round
- `Shop` and `Settings` are placeholder screens for now
- Player paddle controls (`W/S` or `Up/Down`)
- Basic CPU paddle AI
- Ball collision with paddles and walls
- Score tracking to 7 points
- Pause menu (`Esc`) with resume or return to main menu

## Prerequisites

- CMake 3.24+
- A C++20 compiler
- Git (used by CMake `FetchContent` to download raylib)

## Build and Run (Windows PowerShell)

```powershell
Set-Location "C:\Users\Tim\Desktop\Pong2"
cmake -S . -B build
cmake --build build
.\build\Pong2.exe
```

If you use a multi-config generator (for example Visual Studio), run `build\Debug\Pong2.exe` instead.

## Controls

- Main menu navigation: `W/S` or `Up/Down`, `Enter`, or mouse click
- Start game: select `Play`
- Move up: `W` or `Up Arrow`
- Move down: `S` or `Down Arrow`
- Pause / resume: `Esc`
- Pause option confirm: `Enter` or mouse click
- Return to main menu from game over: `M` or `Esc`
- Restart after game over: `R`
- Quit app: close window
