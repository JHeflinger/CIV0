#include "startscene.h"
#include "raylib.h"

void DrawStartScene() {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawText("This is the start scene!", 190, 200, 20, GOLD);

    EndDrawing();
}

void UpdateStartScene() {

}
