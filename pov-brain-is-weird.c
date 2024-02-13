#include <stdio.h>

#include "raylib.h"

int main(void) {
    const int width = 800;
    const int height = 600;

    InitWindow(width, height, "pov: brain is weird");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}