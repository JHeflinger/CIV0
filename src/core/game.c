#include "game.h"
#include "scenes/startscene.h"
#include "scenes/corescene.h"
#include "core/logger.h"
#include "raylib.h"
#include "easynet.h"

GameState g_CurrentState = CORE;

void RunGame() {
	// initialize easynet
	ezn_init();

	// initialize window
	SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1200, 675, "CARDS");

	// main game loop
    while (!WindowShouldClose())
		ManageScenes();

	// close window
    CloseWindow();

	// clean up easynet safely
	if (ezn_safe_clean() == EZN_ERROR) 
		LOG_WARN("Unable to safely clean network resources!");
	ezn_clean();
}

void ManageScenes() {
	// manage game scenes
    switch(g_CurrentState) {
        case TITLE:
			UpdateStartScene();
            DrawStartScene();
            break;
        case CORE:
			UpdateCoreScene();
            DrawCoreScene();
            break;
        default:
            LOG_FATAL("Unhandled scene detected");
            break;
    }
}
