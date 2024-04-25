#ifndef CORESCENE_H
#define CORESCENE_H

#include "utils/datastructs.h"
#include "easynet.h"

#define DEFAULT_CORE_PORT 6969

typedef enum {
	CORE_NONE = 0,
	CORE_INIT,
	CORE_MAIN
} CoreSceneState;

typedef enum {
	FREE_CAMERA,
	FREE_PLAN,
} InteractionState;

void DrawCoreScene();
void DrawCells();
void DrawArtifacts();
void DrawUI();
void UpdateCoreScene();
void MainCoreScene();
void UpdateCells();
void UpdateUser();
void UpdateCoreCamera();
void InitializeCoreScene();
void CleanCoreScene();

#endif
