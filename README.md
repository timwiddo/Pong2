# Pong2 (Raylib)

Simple Pong game in C++ using raylib.

## Project Structure

- `main.cpp` - Application bootstrap (window setup and game start)
- `include/game/Game.h` and `src/game/Game.cpp` - Main game loop and orchestration
- `include/entities/Paddle.h` and `src/entities/Paddle.cpp` - Paddle behavior
- `include/entities/Ball.h` and `src/entities/Ball.cpp` - Ball movement and collisions
- `include/ui/ScoreBoard.h` and `src/ui/ScoreBoard.cpp` - Score handling and drawing

## Features

- Player paddle controls (`W/S` or `Up/Down`)
- Basic CPU paddle AI
- Ball collision with paddles and walls
- Score tracking to 7 points
- Restart flow (`R`) after game over

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

- Move up: `W` or `Up Arrow`
- Move down: `S` or `Down Arrow`
- Restart after game over: `R`
- Quit: `Esc` or close window



