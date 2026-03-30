# Pong2 (Raylib)

Simple Pong game in C++ using raylib.

This version includes a basic coin reward system with local persistence (no database).

## Project Structure

- `main.cpp` - Application bootstrap (window setup and game start)
- `include/game/Game.h` and `src/game/Game.cpp` - Main game loop and orchestration
- `include/entities/Paddle.h` and `src/entities/Paddle.cpp` - Paddle behavior
- `include/entities/Ball.h` and `src/entities/Ball.cpp` - Ball movement and collisions
- `include/ui/ScoreBoard.h` and `src/ui/ScoreBoard.cpp` - Score handling and drawing

## Features

- Home screen with `Play`, `Shop`, and `Settings`
- `Play` starts a fresh Pong round
- `Shop` is a placeholder screen for now
- `Settings` lets you adjust ball size and paddle size with capped ranges
- Player paddle controls (`W/S` or `Up/Down`)
- Basic CPU paddle AI
- Ball collision with paddles and walls
- Score tracking (first to 3 goals wins)
- Pause menu (`P`) with resume or return to main menu
- Coins: `+10` for a win, `+3` for a loss
- Coin HUD: yellow coin icon + total coin count in the top-right corner

## Match Rules and Rewards

- A match ends when either side reaches `3` goals.
- If the player wins the match, they earn `10` coins.
- If the player loses the match, they still earn `3` coins.
- Coins are awarded once per completed match (on game over), not per goal.

## Saved Data (Coins)

- Coins are saved locally on the PC and persist between sessions.
- Windows save file path:
  - `%LOCALAPPDATA%\Pong2\coins.dat`
- Fallback paths used automatically if needed:
  - `%USERPROFILE%\Pong2\coins.dat`
  - `coins.dat` in the current working folder (last resort)

No database or external setup is required.

## Prerequisites

- CMake 3.24+
- A C++20 compiler
- Git (used by CMake `FetchContent` to download raylib)

## Quick Start for Grading (Windows)

If you received a zip that already contains an executable, run that first:

```powershell
# Example locations in this repo setup:
.\build\Pong2.exe
# or
.\cmake-build-debug\Pong2.exe
```

If no runnable executable is included, build from source:

```powershell
Set-Location "C:\Users\Tim\Desktop\Pong2"
cmake -S . -B build
cmake --build build
.\build\Pong2.exe
```

If you use a multi-config generator (for example Visual Studio), run `build\Debug\Pong2.exe` instead.

## Build and Run (Windows PowerShell)

```powershell
Set-Location "C:\Users\Tim\Desktop\Pong2"
cmake -S . -B build
cmake --build build
.\build\Pong2.exe
```


## Controls

- Main menu navigation: `W/S` or `Up/Down`, `Enter`, or mouse click
- Start game: select `Play`
- Move up: `W` or `Up Arrow`
- Move down: `S` or `Down Arrow`
- Pause / resume: `P`
- Pause option confirm: `Enter` or mouse click
- Settings navigation: `W/S` or `Up/Down`
- Settings value adjust: `A/D` or `Left/Right`
- Settings reset to defaults: select `Reset to Default`, then `Enter` or `Space`
- Leave settings: `M`
- Return to main menu from game over: `M` or `Esc`
- Restart after game over: `R`
- Quit app: close window
