#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TILE_SIZE 5
#define ROWS WINDOW_HEIGHT / TILE_SIZE
#define COLS WINDOW_WIDTH / TILE_SIZE
#define CLOCK_STEP PI / 30  // 6 degrees in radians

typedef enum Screen {
    MENU = 0,
    LINES,
    CLOCK,
} Screen;

typedef struct {
    int rows;
    int cols;
    int spacing;
    int titleBarHeight;
    Vector2 selectedTile;
} MenuState;

typedef struct {
    Vector2 p1;
    Vector2 p2;
} LinesState;

typedef struct {
    int radius;
    Vector2 handOrigin;
    Vector2 handDest;
} ClockState;

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

void initGrid(bool grid[ROWS][COLS]) {
    for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
            grid[y][x] = GetRandomValue(0, 1);
        }
    }
}

void drawGrid(bool grid[ROWS][COLS]) {
    for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
            if (grid[y][x])
                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
        }
    }
}

const char *tileNames[] = {"lines", "clock", "placeholder", "placeholder", "placeholder", "placeholder"};
const Screen screens[] = {LINES, CLOCK, MENU, MENU, MENU, MENU};
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

void lineV(bool state[ROWS][COLS], Vector2 p1, Vector2 p2) {
    line(state, p1.x, p1.y, p2.x, p2.y);
}

void circle(bool state[ROWS][COLS], Vector2 origin, int radius) {
    for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
            if (round(sqrt((x - origin.x) * (x - origin.x) + (y - origin.y) * (y - origin.y))) == radius) {
                state[y][x] = !state[y][x];
            }
        }
    }
}

int main(void) {
    SetRandomSeed(time(NULL));

    bool grid[ROWS][COLS] = {false};
    initGrid(grid);

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

    LinesState linesState = {
        .p1 = {0},
        .p2 = {0}};

    ClockState clockState = {
        .radius = ROWS / 2 * 3 / 4,
        .handOrigin = {COLS / 2, ROWS / 2},
        .handDest = {COLS / 2, ROWS / 2 - clockState.radius}};

    bool paused = false;
    unsigned int frameCount = 0;
    while (!WindowShouldClose()) {
        frameCount = (frameCount + 1) % 60;

        switch (currentScreen) {
            case MENU: {
                if (IsKeyPressed(KEY_ENTER)) {
                    short screenIdx = menuState.selectedTile.y * menuState.cols + menuState.selectedTile.x;
                    currentScreen = screens[screenIdx];
                }

                if (IsKeyPressed(KEY_LEFT))
                    menuState.selectedTile.x = euclideanModulo((int)menuState.selectedTile.x - 1, menuState.cols);
                if (IsKeyPressed(KEY_UP))
                    menuState.selectedTile.y = euclideanModulo((int)menuState.selectedTile.y - 1, menuState.rows);
                if (IsKeyPressed(KEY_RIGHT))
                    menuState.selectedTile.x = ((int)menuState.selectedTile.x + 1) % menuState.cols;
                if (IsKeyPressed(KEY_DOWN))
                    menuState.selectedTile.y = ((int)menuState.selectedTile.y + 1) % menuState.rows;
            } break;

            case LINES:
            case CLOCK:
            default: {
                if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;

                if (IsKeyPressed(KEY_P)) paused = !paused;
            } break;
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
                if (!paused && frameCount % 15 == 0) {
                    linesState.p1.x = GetRandomValue(0, COLS);
                    linesState.p1.y = GetRandomValue(0, ROWS);
                    linesState.p2.x = GetRandomValue(0, COLS);
                    linesState.p2.y = GetRandomValue(0, ROWS);
                    lineV(grid, linesState.p1, linesState.p2);
                }

                drawGrid(grid);
            } break;

            case CLOCK: {
                if (!paused && frameCount % 3 == 0) circle(grid, clockState.handOrigin, clockState.radius);

                if (!paused && frameCount == 0) {
                    Vector2 v = {clockState.handDest.x - clockState.handOrigin.x, clockState.handDest.y - clockState.handOrigin.y};
                    v.x = v.x * cos(CLOCK_STEP) - v.y * sin(CLOCK_STEP);
                    v.y = v.x * sin(CLOCK_STEP) + v.y * cos(CLOCK_STEP);

                    clockState.handDest.x = round(clockState.handOrigin.x + v.x);
                    clockState.handDest.y = round(clockState.handOrigin.y + v.y);

                    lineV(grid, clockState.handOrigin, clockState.handDest);
                }

                drawGrid(grid);
            } break;

            default: {
                DrawText("you shouldn't be here",
                         WINDOW_WIDTH / 2 - MeasureText("you shouldn't be here", 20) / 2, WINDOW_HEIGHT / 2 - 10,
                         20, BLACK);
            } break;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}