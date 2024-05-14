#include "game.h"
#include "scenes/corescene.h"
#include "scenes/startscene.h"
#include "core/logger.h"
#include "network/network.h"
#include "raylib.h"

GameState g_CurrentState = TITLE;

// temp
int server_or_client;
void trash_func(int var) { server_or_client = var; }

void RunGame() {
	// disable trivial logs
	SetTraceLogLevel(LOG_ERROR);

	// initialize easynet
	InitializeNetwork(server_or_client);

	// initialize window
	SetConfigFlags(FLAG_VSYNC_HINT);// | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    InitWindow(1400, 900, "CARDS");

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
