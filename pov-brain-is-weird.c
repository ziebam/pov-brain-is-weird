#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TILE_SIZE 10
#define ROWS WINDOW_HEIGHT / TILE_SIZE
#define COLS WINDOW_WIDTH / TILE_SIZE

typedef enum Screen {
    MENU = 0,
    LINES
} Screen;

typedef struct {
    int rows;
    int cols;
    int spacing;
    int titleBarHeight;
    Vector2 selectedTile;
} MenuState;

int getSign(int n) {
    if (n > 0)
        return 1;
    else if (n < 0)
        return -1;
    else
        return 0;
}

int euclideanModulo(int a, int b) {
    return (a % b + b) % b;
}

void initState(bool state[ROWS][COLS]) {
    for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
            state[y][x] = GetRandomValue(0, 1);
        }
    }
}

// Flip the pixels between (x1, y1) and (x2, y2) using Bresenham's algorithm generalized to work
// with any slope. Credit: https://www.uobabylon.edu.iq/eprints/publication_2_22893_6215.pdf.
void line(bool state[ROWS][COLS], int x1, int y1, int x2, int y2) {
    int dx, dy, x, y, e, a, b, s1, s2, swapped = 0, temp;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);

    s1 = getSign(x2 - x1);
    s2 = getSign(y2 - y1);

    if (dy > dx) {
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
    for (int i = 1; i < dx; i++) {
        state[y][x] = !state[y][x];

        if (e < 0) {
            if (swapped)
                y = y + s2;
            else
                x = x + s1;
            e = e + a;
        } else {
            y = y + s2;
            x = x + s1;
            e = e + b;
        }
    }
}

const char *tileNames[] = {"placeholder", "placeholder", "placeholder", "placeholder", "placeholder", "placeholder"};
void drawMenuTiles(MenuState menuState) {
    float outlineWidth = (WINDOW_WIDTH - (menuState.cols + 1) * menuState.spacing) / menuState.cols;
    float outlineHeight = (WINDOW_HEIGHT - menuState.titleBarHeight - (menuState.rows + 1) * menuState.spacing) / menuState.rows;
    for (int i = 0; i < menuState.rows; i++) {
        for (int j = 0; j < menuState.cols; j++) {
            short tileIdx = i * menuState.cols + j;

            float x = menuState.spacing * (j + 1) + outlineWidth * j;
            float y = menuState.titleBarHeight + menuState.spacing * (i + 1) + outlineHeight * i;
            float width = outlineWidth;
            float height = outlineHeight;

            Rectangle tile = {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
            };
            Color tileColor = (i == menuState.selectedTile.y && j == menuState.selectedTile.x) ? MAROON : BLACK;
            DrawRectangleLinesEx(tile, 5.0f, tileColor);

            const char *tileName = tileNames[tileIdx];
            DrawText(tileName,
                     x + width / 2 - MeasureText(tileName, 20) / 2, y + height / 2 - 10,
                     20, BLACK);
        }
    }
}

int main(void) {
    SetRandomSeed(time(NULL));

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "pov: brain is weird");
    SetExitKey(KEY_NULL);
    SetWindowIcon(LoadImage("./resources/pov-you-wake-up-in-poland.png"));
    SetTargetFPS(60);

    Screen currentScreen = MENU;
    MenuState menuState = {
        .rows = 2,
        .cols = 3,
        .spacing = 40,
        .titleBarHeight = 40,
        .selectedTile = {0, 0},
    };

    bool state[ROWS][COLS] = {false};
    initState(state);
    bool paused = false;
    // Input polling is done per frame, so I'm counting the frames manually to slow down the
    // rendering while keeping the controls responsive.
    unsigned int frameCount = 0;
    unsigned short x1, y1, x2, y2;
    while (!WindowShouldClose()) {
        frameCount = (frameCount + 1) % 60;

        switch (currentScreen) {
            case MENU: {
                if (IsKeyPressed(KEY_ENTER)) currentScreen = LINES;

                if (IsKeyPressed(KEY_LEFT))
                    menuState.selectedTile.x = euclideanModulo((int)menuState.selectedTile.x - 1, menuState.cols);
                if (IsKeyPressed(KEY_UP))
                    menuState.selectedTile.y = euclideanModulo((int)menuState.selectedTile.y - 1, menuState.rows);
                if (IsKeyPressed(KEY_RIGHT))
                    menuState.selectedTile.x = ((int)menuState.selectedTile.x + 1) % menuState.cols;
                if (IsKeyPressed(KEY_DOWN))
                    menuState.selectedTile.y = ((int)menuState.selectedTile.y + 1) % menuState.rows;
            } break;

            case LINES: {
                if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;

                if (IsKeyPressed(KEY_P)) paused = !paused;
            } break;

            default:
                break;
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        switch (currentScreen) {
            case MENU: {
                DrawText("pov: brain is weird",
                         WINDOW_WIDTH / 2 - MeasureText("pov: brain is weird", 20) / 2, 10,
                         20, BLACK);

                Vector2 separatorStart = {0, menuState.titleBarHeight};
                Vector2 separatorEnd = {WINDOW_WIDTH, menuState.titleBarHeight};
                DrawLineEx(separatorStart, separatorEnd, 3, BLACK);

                drawMenuTiles(menuState);
            } break;

            case LINES: {
                x1 = GetRandomValue(0, COLS);
                y1 = GetRandomValue(0, ROWS);
                x2 = GetRandomValue(0, COLS);
                y2 = GetRandomValue(0, ROWS);

                if (!paused && frameCount % 15 == 0) line(state, x1, y1, x2, y2);

                for (size_t y = 0; y < ROWS; y++) {
                    for (size_t x = 0; x < COLS; x++) {
                        if (state[y][x])
                            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
                    }
                }
            } break;

            default:
                break;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}