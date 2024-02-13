#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TILE_WIDTH 10
#define TILE_HEIGHT 10
#define ROWS WINDOW_HEIGHT / TILE_HEIGHT
#define COLS WINDOW_WIDTH / TILE_WIDTH

void init_state(bool state[COLS][ROWS])
{
    for (size_t y = 0; y < ROWS; ++y)
    {
        for (size_t x = 0; x < COLS; ++x)
        {
            state[y][x] = GetRandomValue(0, 1);
        }
    }
}

int get_sign(int n)
{
    if (n > 0)
        return 1;
    else if (n < 0)
        return -1;
    else
        return 0;
}

// Flip the pixels between (x1, y1) and (x2, y2) using Bresenham's algorithm generalized to work
// with any slope. Credit: https://www.uobabylon.edu.iq/eprints/publication_2_22893_6215.pdf.
void flip_cells_alongside_line(bool state[COLS][ROWS], int x1, int y1, int x2, int y2)
{
    int dx, dy, x, y, e, a, b, s1, s2, swapped = 0, temp;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);

    s1 = get_sign(x2 - x1);
    s2 = get_sign(y2 - y1);

    if (dy > dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
        swapped = 1;
    }

    e = 2 * dy - dx;
    a = 2 * dy;
    b = 2 * dy - 2 * dx;

    x = x1;
    y = y1;
    for (int i = 1; i < dx; i++)
    {
        state[y][x] = !state[y][x];

        if (e < 0)
        {
            if (swapped)
                y = y + s2;
            else
                x = x + s1;
            e = e + a;
        }
        else
        {
            y = y + s2;
            x = x + s1;
            e = e + b;
        }
    }
}

int main(void)
{
    SetRandomSeed(time(NULL));

    bool state[COLS][ROWS] = {false};
    init_state(state);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "pov: brain is weird");
    SetTargetFPS(60);

    bool paused = false;
    // Input polling is done per frame, so I'm counting the frames manually to slow down the "visualization".
    int frameCount = 0;

    int x1, y1, x2, y2;
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_P))
            paused = !paused;

        x1 = GetRandomValue(0, COLS);
        y1 = GetRandomValue(0, ROWS);
        x2 = GetRandomValue(0, COLS);
        y2 = GetRandomValue(0, ROWS);

        BeginDrawing();
        ClearBackground(WHITE);

        for (size_t y = 0; y < ROWS; y++)
        {
            for (size_t x = 0; x < COLS; x++)
            {
                if (state[y][x] == 1)
                {
                    DrawRectangle(x * TILE_WIDTH, y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                }
            }
        }

        if (!paused && frameCount % 15 == 0)
            flip_cells_alongside_line(state, x1, y1, x2, y2);

        EndDrawing();

        frameCount += 1;
        // Should never happen, but let's prevent integer overflows.
        if (frameCount >= 60)
            frameCount %= 60;
    }

    CloseWindow();

    return 0;
}