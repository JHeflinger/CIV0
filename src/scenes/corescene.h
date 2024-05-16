#ifndef CORESCENE_H
#define CORESCENE_H

#include "utils/datastructs.h"
#include "easynet.h"
#include "raylib.h"

#define DEFAULT_CORE_PORT 6969

typedef enum {
	CORE_NONE = 0,
	CORE_INIT,
	CORE_MAIN
} CoreSceneState;

typedef enum {
	GAME_OK,
	GAME_OVER,
} CoreGameState;

typedef enum {
	FREE_CAMERA,
	FREE_PLAN,
} InteractionState;

int InBox(Vector2 coordinate, int x, int y, int w, int h);
void DrawCoreScene();
void DrawCells();
void DrawArtifacts();
void DrawUI();
void DrawOverlay();
void UpdateCoreScene();
void MainCoreScene();
void UpdateCells();
void UpdateUser();
void UpdateCoreCamera();
void UpdateServer();
void UpdateLeaderboard();
void UpdateState();
void InitializeCoreScene();
void CleanCoreScene();
void CalculateSurroundings(int64_t x, int64_t y, char* result, int* count);
void SetupBoard(int width, int height);
void* GetRawMap();

#endif
