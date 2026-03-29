cmake_minimum_required(VERSION 3.24)
project(Pong2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
	raylib
	GIT_REPOSITORY https://github.com/raysan5/raylib.git
	GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(
	Pong2
	main.cpp
	src/game/Game.cpp
	src/entities/Paddle.cpp
	src/entities/Ball.cpp
	src/ui/ScoreBoard.cpp
)

target_include_directories(Pong2 PRIVATE include)
target_link_libraries(Pong2 PRIVATE raylib)
