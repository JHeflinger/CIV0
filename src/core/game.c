#include "game.h"
#include "scenes/corescene.h"
#include "scenes/startscene.h"
#include "core/logger.h"
#include "network/network.h"
#include "raylib.h"

GameState g_CurrentState = TITLE;

void RunGame() {

	// disable trivial logs
	SetTraceLogLevel(LOG_ERROR);

	// initialize window
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    InitWindow(1400, 900, "Civilization 0");

	// main game loop
    while (!WindowShouldClose())
		ManageScenes();

	// close window
    CloseWindow();

	// clean up network service
	if (!ShutdownNetwork()) LOG_FATAL("Unable to shutdown network service");
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

void ChangeScene(GameState state) {
    g_CurrentState = state;
}