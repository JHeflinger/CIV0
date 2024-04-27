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

CoreSceneState               g_State              = CORE_NONE;
InteractionState             g_InteractionState   = FREE_CAMERA;
Camera2D                     g_Camera             = { 0 };
Cellmap                      g_Map                = { 0 };
ARRLIST_DynamicCoordinate    g_QueuedCells        = { 0 };
uint32_t                     g_Ping               = -1;

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
	// draw active cells
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

	// draw queued cells
	for (size_t i = 0; i < g_QueuedCells.size; i++) {
		Rectangle rect = { g_QueuedCells.data[i].x * CELLSIZE, g_QueuedCells.data[i].y * CELLSIZE, CELLSIZE, CELLSIZE };
		switch (g_QueuedCells.data[i].value) {
			case 'R':
				DrawRectangleRec(rect, GREEN);
				break;
			default:
				LOG_FATAL("Unknown cell detected - unable to handle \'%c\'", g_QueuedCells.data[i].value);
				break;
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
			float relpos = ((float)i) / ((float)gridsize);
			float limiter = sqrt(abs(((float)gridsize*gridsize) - (i*i)));
			DrawLine((origin_x + -1*i)*CELLSIZE, (origin_y + -1*limiter)*CELLSIZE, (origin_x + -1*i)*CELLSIZE, (origin_y + limiter)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + i)*CELLSIZE, (origin_y + -1*limiter)*CELLSIZE, (origin_x + i)*CELLSIZE, (origin_y + limiter)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*limiter)*CELLSIZE, (origin_y + i)*CELLSIZE, (origin_x + limiter)*CELLSIZE, (origin_y + i)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*limiter)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, (origin_x + limiter)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, LIGHTGRAY);
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
	
	// update server (if possible)
	UpdateServer();
}

void UpdateUser() {
	// add queued cells
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && g_InteractionState == FREE_PLAN) {
		Vector2 m_coords = GetScreenToWorld2D(GetMousePosition(), g_Camera);
		DynamicCoordinate coord;
		coord.x = (int64_t)(m_coords.x / CELLSIZE);
		coord.y = (int64_t)(m_coords.y / CELLSIZE);
		coord.value = 'R';
		if (!ARRLIST_DynamicCoordinate_has(&g_QueuedCells, coord)) 
			ARRLIST_DynamicCoordinate_add(&g_QueuedCells, coord);
	}

	// send queued cells
	if (IsKeyReleased(KEY_ENTER) && g_InteractionState == FREE_PLAN) {
		if (GetNetworkType() == SERVER) {
			for (size_t i = 0; i < g_QueuedCells.size; i++)
				if (GetCell(&g_Map, g_QueuedCells.data[i].x, g_QueuedCells.data[i].y, g_QueuedCells.data[i].value) == '\0')
					AddCell(&g_Map, g_QueuedCells.data[i].x, g_QueuedCells.data[i].y, g_QueuedCells.data[i].value);
		} else if (GetNetworkType() == CLIENT) {
			SendPacket('q', (EZN_BYTE*)g_QueuedCells.data, g_QueuedCells.size * sizeof(DynamicCoordinate));
		} else {
			LOG_FATAL("Unknown network type detected");
		}
		ARRLIST_DynamicCoordinate_clear(&g_QueuedCells);
	}

	// switch planning states
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

void UpdateServer() {
	if (GetNetworkType() == SERVER) {
		
	}
}

void CleanCoreScene() {

}
