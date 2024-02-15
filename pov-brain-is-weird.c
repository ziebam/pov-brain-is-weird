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

typedef enum Screen
{
    MENU = 0,
    LINES
} Screen;

typedef struct
{
    int rows;
    int cols;
    int spacing;
    int titleBarHeight;
} MenuState;

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
void line(bool state[COLS][ROWS], int x1, int y1, int x2, int y2)
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

void draw_menu_tiles(MenuState menuState)
{
    float outlineWidth = (WINDOW_WIDTH - (menuState.cols + 1) * menuState.spacing) / menuState.cols;
    float outlineHeight = (WINDOW_HEIGHT - menuState.titleBarHeight - (menuState.rows + 1) * menuState.spacing) / menuState.rows;
    for (int i = 0; i < menuState.cols; i++)
    {
        for (int j = 0; j < menuState.rows; j++)
        {
            Rectangle outline = {
                .x = menuState.spacing * (i + 1) + outlineWidth * i,
                .y = menuState.titleBarHeight + menuState.spacing * (j + 1) + outlineHeight * j,
                .width = outlineWidth,
                .height = outlineHeight,
            };
            DrawRectangleLinesEx(outline, 5.0f, MAROON);
        }
    }
}

int main(void)
{
    SetRandomSeed(time(NULL));

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "pov: brain is weird");
    SetExitKey(KEY_NULL);
    Image icon = LoadImage("./resources/pov-you-wake-up-in-poland.png");
    SetWindowIcon(icon);
    SetTargetFPS(60);

    Screen currentScreen = MENU;
    MenuState menuState = {
        .rows = 4,
        .cols = 4,
        .spacing = 50,
        .titleBarHeight = 40,
    };

    bool state[COLS][ROWS] = {false};
    init_state(state);
    bool paused = false;
    // Input polling is done per frame, so I'm counting the frames manually to slow down the
    // rendering while keeping the controls responsive.
    unsigned int frameCount = 0;
    unsigned short x1, y1, x2, y2;
    while (!WindowShouldClose())
    {
        frameCount = (frameCount + 1) % 60;

        switch (currentScreen)
        {
        case MENU:
        {
            if (IsKeyPressed(KEY_ENTER))
                currentScreen = LINES;
        }
        break;

        case LINES:
        {
            if (IsKeyPressed(KEY_P))
                paused = !paused;

            if (IsKeyPressed(KEY_ESCAPE))
                currentScreen = MENU;
        }
        break;

        default:
            break;
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        switch (currentScreen)
        {
        case MENU:
        {
            DrawText("pov: brain is weird",
                     WINDOW_WIDTH / 2 - MeasureText("pov: brain is weird", 20) / 2, 10,
                     20, BLACK);

            Vector2 separatorStart = {0, menuState.titleBarHeight};
            Vector2 separatorEnd = {WINDOW_WIDTH, menuState.titleBarHeight};
            DrawLineEx(separatorStart, separatorEnd, 3, BLACK);

            draw_menu_tiles(menuState);
        }
        break;

        case LINES:
        {
            x1 = GetRandomValue(0, COLS);
            y1 = GetRandomValue(0, ROWS);
            x2 = GetRandomValue(0, COLS);
            y2 = GetRandomValue(0, ROWS);

            if (!paused && frameCount % 15 == 0)
                line(state, x1, y1, x2, y2);

            for (size_t y = 0; y < ROWS; y++)
            {
                for (size_t x = 0; x < COLS; x++)
                {
                    if (state[y][x])
                    {
                        DrawRectangle(x * TILE_WIDTH, y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                    }
                }
            }
        }
        break;

        default:
            break;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}