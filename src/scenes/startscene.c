#include "startscene.h"
#include "raylib.h"

void UpdateStartScene() {

}

void DrawStartScene() {
    BeginDrawing();

    ClearBackground(CLITERAL(Color){ 40, 40, 40, 255 });

    DrawText("Jason Heflinger's", (GetScreenWidth() - 260) / 2, (GetScreenHeight() - 400) / 2, 30, RAYWHITE);
    DrawText("CIVILIZATION 0", (GetScreenWidth() - 740) / 2, (GetScreenHeight() - 300) / 2, 96, RAYWHITE);

    int xplus = (GetScreenWidth() - 300) / 2;
    int yplus = (GetScreenHeight() - 50) / 2;
    DrawRectangle(10 + xplus, 10 + yplus, 300, 60, DARKGRAY);
    DrawRectangle(0 + xplus, 0 + yplus, 300, 60, GRAY);
    DrawText("HOST", 100 + xplus, 10 + yplus, 38, RAYWHITE);

    DrawRectangle(10 + xplus, 110 + yplus, 300, 60, DARKGRAY);
    DrawRectangle(0 + xplus, 100 + yplus, 300, 60, GRAY);
    DrawText("JOIN", 105 + xplus, 110 + yplus, 38, RAYWHITE);

    DrawRectangle(10 + xplus, 210 + yplus, 300, 60, DARKGRAY);
    DrawRectangle(0 + xplus, 200 + yplus, 300, 60, GRAY);
    DrawText("QUIT", 105 + xplus, 210 + yplus, 38, RAYWHITE);

    EndDrawing();
}