#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
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
    DVD,
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

typedef struct {
    int width;
    int height;
    bool *mask;
    Vector2 direction;
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
    Vector2 p4;
} DvdState;

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

const char *tileNames[] = {"lines", "clock", "dvd", "placeholder", "placeholder", "placeholder"};
const Screen screens[] = {LINES, CLOCK, DVD, MENU, MENU, MENU};
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
void line(bool grid[ROWS][COLS], int x1, int y1, int x2, int y2) {
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
        grid[y][x] = !grid[y][x];

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

void lineV(bool grid[ROWS][COLS], Vector2 p1, Vector2 p2) {
    line(grid, p1.x, p1.y, p2.x, p2.y);
}

void rectangle(bool grid[ROWS][COLS], Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4) {
    lineV(grid, p1, p2);
    lineV(grid, p2, p3);
    lineV(grid, p3, p4);
    lineV(grid, p4, p1);
}

DvdState parseDvdFile(DvdState *dvdState) {
    Nob_String_Builder sbContent = {0};
    if (!nob_read_entire_file("./resources/dvd.ppm", &sbContent)) exit(1);
    Nob_String_View svContent = nob_sv_from_parts(sbContent.items, sbContent.count);

    size_t linesCount = 0;
    bool *mask;
    for (; svContent.count > 0; ++linesCount) {
        Nob_String_View line = nob_sv_chop_by_delim(&svContent, '\n');
        // Skip the header and the max value line.
        if (linesCount == 0 || linesCount == 2) continue;

        if (linesCount == 1) {
            dvdState->width = strtol(nob_sv_chop_by_delim(&line, ' ').data, NULL, 10);
            dvdState->height = strtol(line.data, NULL, 10);
            if (dvdState->width == 0 || dvdState->height == 0) {
                nob_log(NOB_ERROR, "Unexpected dimension in the dvd.ppm file.");
                exit(1);
            }
            mask = malloc(dvdState->width * dvdState->height * sizeof(bool));
        } else {
            for (int i = 0; i < dvdState->width; i++) {
                int r = strtol(nob_sv_chop_by_delim(&line, ' ').data, NULL, 10);
                int g = strtol(nob_sv_chop_by_delim(&line, ' ').data, NULL, 10);

                int b;
                if (i == dvdState->width - 1) {
                    b = strtol(line.data, NULL, 10);
                } else {
                    b = strtol(nob_sv_chop_by_delim(&line, ' ').data, NULL, 10);
                }

                if (r != g && g != b) {
                    nob_log(NOB_ERROR, "Unexpected color in the dvd.ppm file.");
                    exit(1);
                }

                mask[(linesCount - 3) * dvdState->width + i] = r == 255;
            }
        }
    }

    dvdState->mask = mask;
    return *dvdState;
}

void dvd(bool grid[ROWS][COLS], DvdState dvdState) {
    for (size_t y = dvdState.p1.y; y < dvdState.p3.y; y++) {
        for (size_t x = dvdState.p1.x; x < dvdState.p2.x; x++) {
            int maskX = x - dvdState.p1.x;
            int maskY = y - dvdState.p1.y;

            if (dvdState.mask[dvdState.width * maskY + maskX]) grid[y][x] = !grid[y][x];
        }
    }
}

void circle(bool grid[ROWS][COLS], Vector2 origin, int radius) {
    for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
            if (round(sqrt((x - origin.x) * (x - origin.x) + (y - origin.y) * (y - origin.y))) == radius) {
                grid[y][x] = !grid[y][x];
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

    DvdState dvdState = {
        .mask = NULL,
        .direction = {1, 1},
        .p1 = {COLS / 2, ROWS / 2},
        .p2 = {COLS / 2, ROWS / 2},
        .p3 = {COLS / 2, ROWS / 2},
        .p4 = {COLS / 2, ROWS / 2},
    };
    parseDvdFile(&dvdState);
    dvdState.p2.x += dvdState.width;
    dvdState.p2.y += dvdState.height;
    dvdState.p3.x += dvdState.width;
    dvdState.p3.y += dvdState.height;
    dvdState.p4.x += dvdState.width;
    dvdState.p4.y += dvdState.height;

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
            case DVD:
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

            case DVD: {
                if (!paused && frameCount % 2 == 0) {
                    // collision checks
                    // top
                    if (dvdState.p1.y == 0 || dvdState.p2.y == 0)
                        dvdState.direction.y = 1;
                    // // right
                    if (dvdState.p2.x == COLS - 1 || dvdState.p3.x == COLS - 1)
                        dvdState.direction.x = -1;
                    // // bottom
                    if (dvdState.p3.y == ROWS - 1 || dvdState.p4.y == ROWS - 1)
                        dvdState.direction.y = -1;
                    // // left
                    if (dvdState.p1.x == 0 || dvdState.p4.x == 0)
                        dvdState.direction.x = 1;

                    dvdState.p1.x += dvdState.direction.x;
                    dvdState.p1.y += dvdState.direction.y;
                    dvdState.p2.x += dvdState.direction.x;
                    dvdState.p2.y += dvdState.direction.y;
                    dvdState.p3.x += dvdState.direction.x;
                    dvdState.p3.y += dvdState.direction.y;
                    dvdState.p4.x += dvdState.direction.x;
                    dvdState.p4.y += dvdState.direction.y;
                    dvd(grid, dvdState);
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