#include "corescene.h"
#include "core/logger.h"
#include "core/input.h"
#include "utils/timeutils.h"
#include "network/network.h"
#include "utils/numbers.h"
#include "data/baked.h"
#include "data/gconfig.h"
#include "data/cellmap.h"
#include "raylib.h"
#include <math.h>

CoreSceneState        g_State              = CORE_NONE;
InteractionState      g_InteractionState   = FREE_CAMERA;
Camera2D              g_Camera             = { 0 };
uint32_t              g_Ping               = -1;
Cellmap               g_Map                = { 0 };

void DrawCoreScene() {
    BeginDrawing();

    ClearBackground(DARKGRAY);
    
	BeginMode2D(g_Camera);

	DrawCells();

	DrawArtifacts();

	EndMode2D();

	DrawUI();

    EndDrawing();
}

void DrawCells() {
	char cell_id;
	for (size_t y = 0; y < g_Map.height; y++) {
		for (size_t x = 0; x < g_Map.width; x++) {
			cell_id = g_Map.data[x][y];
			if (cell_id != '\0') {
				Rectangle rect = { (float)(g_Map.x + (int64_t)x) * CELLSIZE, (float)(g_Map.y + (int64_t)y) * CELLSIZE, CELLSIZE, CELLSIZE };
				switch (cell_id) {
					case 'R':
						DrawRectangleRec(rect, RED);
						break;
					default:
						LOG_FATAL("Unknown cell detected - unable to handle \'%c\'", cell_id);
						break;
				}
			}
		}
	}
}

void DrawArtifacts() {
	// draw grid
	if (g_InteractionState == FREE_PLAN) {
		int gridsize = 25;
		int origin_x = g_Camera.target.x / CELLSIZE;
		int origin_y = g_Camera.target.y / CELLSIZE;
		for (int i = 0; i < gridsize; i++) {
			DrawLine((origin_x + -1*i)*CELLSIZE, (origin_y + -1*gridsize)*CELLSIZE, (origin_x + -1*i)*CELLSIZE, (origin_y + gridsize)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + i)*CELLSIZE, (origin_y + -1*gridsize)*CELLSIZE, (origin_x + i)*CELLSIZE, (origin_y + gridsize)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*gridsize)*CELLSIZE, (origin_y + i)*CELLSIZE, (origin_x + gridsize)*CELLSIZE, (origin_y + i)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*gridsize)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, (origin_x + gridsize)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, LIGHTGRAY);
		}
	}
}

void DrawUI() {
	// fps monitor
	char buffer[1024];
	sprintf(buffer, "FPS: %d", (int)(1.0f/GetFrameTime()));
	DrawText(buffer, 10, 10, 18, RAYWHITE);

	//ping monitor
	sprintf(buffer, "PING: %u", g_Ping);
	DrawText(buffer, 10, 30, 18, RAYWHITE);
}

void UpdateCoreScene() {
	// handle scene states
	switch(g_State) {
		case CORE_NONE:
			g_State = CORE_INIT;
			break;
		case CORE_INIT:
			InitializeCoreScene();
			break;
		case CORE_MAIN:
			MainCoreScene();
			break;
		default:
			LOG_FATAL("Unhandled core scene state detected");	
	}
}

void InitializeCoreScene() {
	// initialize network service
	if (GetNetworkType() == SERVER) {
		StartServer();
	} else if (GetNetworkType() == CLIENT) {
		ConnectClient();
	} else {
		LOG_FATAL("Unknown network type detected");
	}

	// initialize camera
    g_Camera.target = (Vector2){ 0.0f, 0.0f };
    g_Camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    g_Camera.rotation = 0.0f;
    g_Camera.zoom = 1.0f;

	// erm, what!
	AddCell(&g_Map, 0, -100, 'R');
	AddCell(&g_Map, 0, 100, 'R');
	AddCell(&g_Map, 100, 0, 'R');
	AddCell(&g_Map, -100, 0, 'R');
	AddCell(&g_Map, 0, 1, 'R');
	AddCell(&g_Map, 1, 2, 'R');
	AddCell(&g_Map, 2, 2, 'R');
	AddCell(&g_Map, 2, 0, 'R');
	AddCell(&g_Map, 2, 1, 'R');

	// change state
	g_State = CORE_MAIN;
}

void MainCoreScene() {
	// update input modules
	UpdateSmartInput();

	// update camera
	UpdateCoreCamera();

	// update cells
	UpdateCells();

	// update user input
	UpdateUser();
}

void UpdateUser() {
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		Vector2 m_coords = GetScreenToWorld2D(GetMousePosition(), g_Camera);
		AddCell(&(g_Map), (int64_t)(m_coords.x / CELLSIZE), (int64_t)(m_coords.y / CELLSIZE), 'R');
	}
	if (IsKeyReleased(KEY_P)) g_InteractionState = g_InteractionState == FREE_CAMERA ? FREE_PLAN : FREE_CAMERA;
}

void UpdateCells() {
	if (GetNetworkType() == SERVER) {
		static float time;
		time += GetFrameTime();
		if (time < CYCLE) return;
		else time = 0.0f;
		uint8_t cell_id;
		ARRLIST_Coordinate changed = { 0 };
		for (size_t y = 0; y < g_Map.height; y++) {
			for (size_t x = 0; x < g_Map.width; x++) {
				cell_id = g_Map.data[x][y];
				int num_neighbors = 0;
				for (int i = -1; i < 2; i++)
					for (int j = -1; j < 2; j++)
						if ((i != 0 || j != 0) && (x + i) >= 0 && (y + j) >= 0 && (x + i) < g_Map.width && (y + j) < g_Map.height)
							if (g_Map.data[x + i][y + j] >= 'A' + DEATH_MARK || (g_Map.data[x + i][y + j] <= 'Z' && g_Map.data[x + i][y + j] >= 'A'))
								num_neighbors++;
				if (cell_id != '\0') {
					if (num_neighbors < 2 || num_neighbors > 3) g_Map.data[x][y] += DEATH_MARK;
				} else {
					if (num_neighbors == 3) g_Map.data[x][y] = 'r';
				}
			}
		}
		for (size_t y = 0; y < g_Map.height; y++) {
			for (size_t x = 0; x < g_Map.width; x++) {
				cell_id = g_Map.data[x][y];
				Coordinate coord;
				coord.x = x;
				coord.y = y;
				if (cell_id >= 'A' + DEATH_MARK) {
					g_Map.data[x][y] = '\0'; // marked for death
					coord.value = '\0';
					ARRLIST_Coordinate_add(&changed, coord);
				}
				else if (cell_id >= 'a') {
					g_Map.data[x][y] = cell_id - ('a' - 'A'); // marked for life
					coord.value = cell_id - ('a' - 'A');
					ARRLIST_Coordinate_add(&changed, coord);
				}
			}
		}
		DistributeData((EZN_BYTE*)changed.data, changed.size * sizeof(Coordinate));
		ARRLIST_Coordinate_clear(&changed);
	} else if (GetNetworkType() == CLIENT) {
		int found_data = TRUE;
		Coordinate coord = { 0 };
		while (found_data) {
			GrabData((EZN_BYTE*)&coord, sizeof(Coordinate), &found_data);
			if (found_data) {
				g_Map.data[coord.x][coord.y] = coord.value;
			}
		}
	} else {
		LOG_FATAL("Unknown getwork type");
	}
}

void UpdateCoreCamera() {
	if (g_InteractionState != FREE_CAMERA && g_InteractionState != FREE_PLAN) return;
	static Vector2 mousepos_old;
	float speed = CELLSIZE * 0.5;
	Vector2 vel = { 0 };
	switch (PeekWS()) {
		case 'w':
			vel.y = -1.0f;
			break;
		case 's':
			vel.y = 1.0f;
			break;
		default:
			vel.y = 0;
	}
	switch (PeekAD()) {
		case 'd':
			vel.x = 1.0f;
			break;
		case 'a':
			vel.x = -1.0f;
			break;
		default:
			vel.x = 0;
	}
	if (vel.x != 0 && vel.y != 0) {
		vel.x /= SQRT2;
		vel.y /= SQRT2;
	}
	g_Camera.target.x += vel.x * speed;
	g_Camera.target.y += vel.y * speed;
	g_Camera.zoom += GetMouseWheelMove() * 0.1f;
	g_Camera.zoom = g_Camera.zoom < 0.1f ? 0.1f : g_Camera.zoom;

	Vector2 currmousepos = GetMousePosition();
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		g_Camera.target.x -= (currmousepos.x - mousepos_old.x) / g_Camera.zoom;
		g_Camera.target.y -= (currmousepos.y - mousepos_old.y) / g_Camera.zoom;
	}
	mousepos_old = currmousepos;
}

void CleanCoreScene() {

}
