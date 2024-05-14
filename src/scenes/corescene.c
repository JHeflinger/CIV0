#include "corescene.h"
#include "core/logger.h"
#include "core/input.h"
#include "utils/timeutils.h"
#include "network/network.h"
#include "utils/numbers.h"
#include "data/baked.h"
#include "data/gconfig.h"
#include "data/cellmap.h"
#include "network/teams.h"
#include "raylib.h"
#include <math.h>

CoreSceneState               g_State              = CORE_NONE;
InteractionState             g_InteractionState   = FREE_CAMERA;
Camera2D                     g_Camera             = { 0 };
Cellmap                      g_Map                = { 0 };
ARRLIST_DynamicCoordinate    g_QueuedCells        = { 0 };
uint32_t                     g_Ping               = -1;
char                         g_ID                 = 'A';
uint64_t                     g_AvailableCells     = 100;
uint64_t                     g_CapturedCells[26];
uint64_t                     g_Leaderboard[26];
CoreGameState                g_GameState          = GAME_OK;
int                          g_BoardWidth         = 100;
int                          g_BoardHeight        = 100;

void DrawCoreScene() {
    BeginDrawing();

    ClearBackground(CLITERAL(Color){ 40, 40, 40, 255 });
    
	BeginMode2D(g_Camera);

	DrawCells();

	DrawArtifacts();

	EndMode2D();

	DrawUI();

	DrawOverlay();

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
				DrawRectangleRec(rect, GetIDColor(cell_id));
			}
		}
	}

	// draw queued cells
	for (size_t i = 0; i < g_QueuedCells.size; i++) {
		Rectangle rect = { g_QueuedCells.data[i].x * CELLSIZE, g_QueuedCells.data[i].y * CELLSIZE, CELLSIZE, CELLSIZE };
		DrawRectangleRec(rect, GREEN);
	}
}

void DrawArtifacts() {
	// blueprint mode
	if (g_InteractionState == FREE_PLAN) {
		// draw grid
		int gridsize = 25;
		int origin_x = g_Camera.target.x / CELLSIZE;
		int origin_y = g_Camera.target.y / CELLSIZE;
		for (int i = 0; i < gridsize; i++) {
			float limiter = sqrt(abs(((float)gridsize*gridsize) - (i*i)));
			DrawLine((origin_x + -1*i)*CELLSIZE, (origin_y + -1*limiter)*CELLSIZE, (origin_x + -1*i)*CELLSIZE, (origin_y + limiter)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + i)*CELLSIZE, (origin_y + -1*limiter)*CELLSIZE, (origin_x + i)*CELLSIZE, (origin_y + limiter)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*limiter)*CELLSIZE, (origin_y + i)*CELLSIZE, (origin_x + limiter)*CELLSIZE, (origin_y + i)*CELLSIZE, LIGHTGRAY);
			DrawLine((origin_x + -1*limiter)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, (origin_x + limiter)*CELLSIZE, (origin_y + -1*i)*CELLSIZE, LIGHTGRAY);
		}

		// draw cell 2 queue 
		Vector2 m_coords = GetScreenToWorld2D(GetMousePosition(), g_Camera);
		if (m_coords.y < 0) m_coords.y -= CELLSIZE;
		if (m_coords.x < 0) m_coords.x -= CELLSIZE;
		m_coords.x = CELLSIZE * ((int)(m_coords.x / CELLSIZE));
		m_coords.y = CELLSIZE * ((int)(m_coords.y / CELLSIZE));
		Rectangle rec = { m_coords.x, m_coords.y, CELLSIZE, CELLSIZE };
		DrawRectangleRec(rec, (g_QueuedCells.size < g_AvailableCells) && 
			(m_coords.x >= (g_BoardWidth * -0.5f * CELLSIZE) && 
			m_coords.x <= (g_BoardWidth * 0.5f * CELLSIZE) && 
			m_coords.y >= (g_BoardHeight * -0.5f * CELLSIZE) && 
			m_coords.y <= (g_BoardHeight * 0.5f * CELLSIZE)
			) ? YELLOW : RED);
	}

	// draw map bounds
	float thickness = 10.0f;
	DrawLineEx(CLITERAL(Vector2){-1 * (CELLSIZE*g_BoardWidth/2), -1 * (CELLSIZE*g_BoardHeight/2)}, CLITERAL(Vector2){(CELLSIZE*((g_BoardWidth/2)+1)), -1 * (CELLSIZE*g_BoardHeight/2)}, thickness, RAYWHITE);
	DrawLineEx(CLITERAL(Vector2){-1 * (CELLSIZE*g_BoardWidth/2), -1 * (CELLSIZE*g_BoardHeight/2)}, CLITERAL(Vector2){-1 * (CELLSIZE*g_BoardWidth/2), (CELLSIZE*((g_BoardHeight/2)+1))}, thickness, RAYWHITE);
	DrawLineEx(CLITERAL(Vector2){(CELLSIZE*((g_BoardWidth/2)+1)), -1 * (CELLSIZE*g_BoardHeight/2)}, CLITERAL(Vector2){(CELLSIZE*((g_BoardWidth/2)+1)), (CELLSIZE*((g_BoardHeight/2)+1))}, thickness, RAYWHITE);
	DrawLineEx(CLITERAL(Vector2){-1 * (CELLSIZE*g_BoardWidth/2), (CELLSIZE*((g_BoardHeight/2)+1))}, CLITERAL(Vector2){(CELLSIZE*((g_BoardWidth/2)+1)), (CELLSIZE*((g_BoardHeight/2)+1))}, thickness, RAYWHITE);
}

void DrawUI() {
	char buffer[1024];
	
	// available cells
	sprintf(buffer, "AVAILABLE CELLS:");
	DrawText(buffer, 20, 20, 18, RAYWHITE);
	sprintf(buffer, "%lu", (unsigned long)g_AvailableCells);
	DrawText(buffer, 190, 20, 18, g_AvailableCells >= 100 ? GREEN : g_AvailableCells < 10 ? RED : YELLOW);

	// fps monitor
	sprintf(buffer, "FPS: %d", (int)(1.0f/GetFrameTime()));
	//DrawText(buffer, 20, 50, 18, RAYWHITE);

	//helper ui
	if (g_InteractionState != FREE_PLAN) {
		sprintf(buffer, "P");
		DrawRectangle(GetScreenWidth() - 53, 13, 40, 40, DARKGRAY);
		DrawRectangle(GetScreenWidth() - 50, 10, 40, 40, GRAY);
		DrawText(buffer, GetScreenWidth() - 40, 18, 28, LIGHTGRAY);
		sprintf(buffer, "Blueprint mode");
		DrawText(buffer, GetScreenWidth() - 190, 22, 18, RAYWHITE);
	} else {
		sprintf(buffer, "P");
		DrawRectangle(GetScreenWidth() - 53, 13, 40, 40, DARKGRAY);
		DrawRectangle(GetScreenWidth() - 50, 10, 40, 40, GRAY);
		DrawText(buffer, GetScreenWidth() - 40, 18, 28, LIGHTGRAY);
		sprintf(buffer, "Exit blueprint mode");
		DrawText(buffer, GetScreenWidth() - 229, 22, 18, RAYWHITE);
	}

	// show team
	DrawRectangle(20, GetScreenHeight() - 70, 50, 50, WHITE);
	DrawRectangle(25, GetScreenHeight() - 65, 40, 40, GetIDColor(g_ID));

	// leaderboard
	for (size_t i = 0; i < 26; i++) {
		size_t max = 0;
		uint64_t maxval = 0;
		for (size_t j = 0; j < 26; j++) {
			if (g_Leaderboard[j] > maxval) {
				max = j;
				maxval = g_Leaderboard[j];
			}
		}
		if (maxval == 0) break;
		DrawRectangle(20, 50 + (i * 22), 15, 15, GetIDColor(max + 'A'));
		if (max + 'A' == (size_t)g_ID)
			DrawRectangle(10, 50 + (i * 22), 5, 15, RAYWHITE);
		sprintf(buffer, i == 0 ? "%dst place - %lu" : (i == 1 ? "%dnd place - %lu" : (i == 2 ? "%drd place - %lu" : "%dth place - %lu")), (int)(i + 1), maxval);
		DrawText(buffer, 45, 50 + (i * 22), 18, i == 0 ? GOLD : (i == 1 ? CLITERAL(Color){ 204, 213, 217, 255 } : (i == 2 ? CLITERAL(Color){ 173, 115, 7, 255 } : RAYWHITE)));
		g_Leaderboard[max] = 0;
	}
}

void DrawOverlay() {
	char buffer[1024];
	if (g_GameState == GAME_OVER) {
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){ 0, 0, 0, 150 });
		sprintf(buffer, "GAME OVER");
		DrawText(buffer, (GetScreenWidth() / 2) - 255 + 10, (GetScreenHeight() / 2) - 37 - 80, 80, RAYWHITE);
		DrawRectangle((GetScreenWidth() / 2) - 115, (GetScreenHeight() / 2) + 70 - 80, 250, 50, DARKGRAY);
		DrawRectangle((GetScreenWidth() / 2) - 125, (GetScreenHeight() / 2) + 60 - 80, 250, 50, GRAY);
		sprintf(buffer, "HOME");
		DrawText(buffer, (GetScreenWidth() / 2) - 45, (GetScreenHeight() / 2) + 70 - 80, 30, RAYWHITE);
		DrawRectangle((GetScreenWidth() / 2) - 115, (GetScreenHeight() / 2) + 150 - 80, 250, 50, DARKGRAY);
		DrawRectangle((GetScreenWidth() / 2) - 125, (GetScreenHeight() / 2) + 140 - 80, 250, 50, GRAY);
		sprintf(buffer, "RE-JOIN");
		DrawText(buffer, (GetScreenWidth() / 2) - 60, (GetScreenHeight() / 2) + 150 - 80, 30, RAYWHITE);
	}
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
		g_ID = ConnectClient();
	} else {
		LOG_FATAL("Unknown network type detected");
	}

	// initialize camera
    g_Camera.target = (Vector2){ 0.0f, 0.0f };
    g_Camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    g_Camera.rotation = 0.0f;
    g_Camera.zoom = 1.0f;

	// map setup
	SetMapBounds(&g_Map, g_BoardWidth/2, g_BoardHeight/2);
	AddCell(&g_Map, 0, -1 * (g_BoardHeight / 2), g_ID);
	AddCell(&g_Map, 0, (g_BoardHeight / 2), g_ID);
	AddCell(&g_Map, -1 * (g_BoardWidth / 2), 0, g_ID);
	AddCell(&g_Map, (g_BoardWidth / 2), 0, g_ID);
	RemoveCell(&g_Map, 0, -1 * (g_BoardHeight / 2));
	RemoveCell(&g_Map, 0, (g_BoardHeight / 2));
	RemoveCell(&g_Map, -1 * (g_BoardWidth / 2), 0);
	RemoveCell(&g_Map, (g_BoardWidth / 2), 0);

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

	// update leaderboard
	UpdateLeaderboard();

	// update user input
	UpdateUser();
	
	// update server (if possible)
	UpdateServer();

	// update state 
	UpdateState();
}

void UpdateUser() {
	// add queued cells
	Vector2 m_coords = GetScreenToWorld2D(GetMousePosition(), g_Camera);
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && g_InteractionState == FREE_PLAN && g_QueuedCells.size < g_AvailableCells && 
		(m_coords.x >= (g_BoardWidth * -0.5f * CELLSIZE) && 
		m_coords.x <= (g_BoardWidth * 0.5f * CELLSIZE) && 
		m_coords.y >= (g_BoardHeight * -0.5f * CELLSIZE) && 
		m_coords.y <= (g_BoardHeight * 0.5f * CELLSIZE)
		)) {
		if (m_coords.y < 0) m_coords.y -= CELLSIZE;
		if (m_coords.x < 0) m_coords.x -= CELLSIZE;
		DynamicCoordinate coord;
		coord.x = (int64_t)(m_coords.x / CELLSIZE);
		coord.y = (int64_t)(m_coords.y / CELLSIZE);
		coord.value = g_ID;
		if (!ARRLIST_DynamicCoordinate_has(&g_QueuedCells, coord)) 
			ARRLIST_DynamicCoordinate_add(&g_QueuedCells, coord);
	}

	// send queued cells
	if (IsKeyReleased(KEY_ENTER) && g_InteractionState == FREE_PLAN && g_QueuedCells.size != 0) {
		for (size_t i = 0; i < g_QueuedCells.size; i++)
			if (GetCell(&g_Map, g_QueuedCells.data[i].x, g_QueuedCells.data[i].y) == '\0')
				AddCell(&g_Map, g_QueuedCells.data[i].x, g_QueuedCells.data[i].y, g_QueuedCells.data[i].value);
		if (GetNetworkType() == CLIENT) SendPacket('q', (EZN_BYTE*)g_QueuedCells.data, g_QueuedCells.size * sizeof(DynamicCoordinate));
		g_AvailableCells -= g_QueuedCells.size;
		if (GetNetworkType() == SERVER) {
			ARRLIST_Coordinate changed = { 0 };
			for (size_t i = 0; i < g_QueuedCells.size; i++) {
				Coordinate coord;
				coord.x = (size_t)(g_QueuedCells.data[i].x - g_Map.x);
				coord.y = (size_t)(g_QueuedCells.data[i].y - g_Map.y);
				coord.value = g_QueuedCells.data[i].value;
				ARRLIST_Coordinate_add(&changed, coord);
			}
			DistributeData((EZN_BYTE*)changed.data, changed.size * sizeof(Coordinate));
			ARRLIST_Coordinate_clear(&changed);
		}
		ARRLIST_DynamicCoordinate_clear(&g_QueuedCells);
	}

	// switch planning states
	if (IsKeyReleased(KEY_P)) {
		if (g_InteractionState == FREE_PLAN) 
			ARRLIST_DynamicCoordinate_clear(&g_QueuedCells);
		g_InteractionState = g_InteractionState == FREE_CAMERA ? FREE_PLAN : FREE_CAMERA;
	}

	if (g_GameState == GAME_OVER) {
		Vector2 mouse_coords = GetMousePosition();
		if (mouse_coords.x > (GetScreenWidth() / 2) - 125 && 
			mouse_coords.x < (GetScreenWidth() / 2) - 125 + 250 &&
			mouse_coords.y > (GetScreenHeight() / 2) + 60 - 80 &&
			mouse_coords.y < (GetScreenHeight() / 2) + 60 - 80 + 50) {
			// home button
		} else if (mouse_coords.x > (GetScreenWidth() / 2) - 125 && 
			mouse_coords.x < (GetScreenWidth() / 2) - 125 + 250 &&
			mouse_coords.y > (GetScreenHeight() / 2) + 140 - 80 &&
			mouse_coords.y < (GetScreenHeight() / 2) + 140 - 80 + 50 &&
			IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			g_GameState = GAME_OK;
			g_AvailableCells = 100;
			g_ID = 'A';
			for (size_t i = 0; i < 26; i++) {
				if (g_Leaderboard[i] == 0) {
					g_ID += i;
					break;
				}
			}
		}
	}
}

void UpdateCells() {
	if (GetNetworkType() == SERVER) {
		memset(g_CapturedCells, 0, sizeof(uint64_t)*26);
		static float time;
		time += GetFrameTime();
		if (time < CYCLE) return;
		else time = 0.0f;
		uint8_t cell_id;
		ARRLIST_Coordinate changed = { 0 };
		for (int64_t y = 0 - NO_BOUNDS; y < (int64_t)g_Map.height + NO_BOUNDS; y++) {
			for (int64_t x = 0 - NO_BOUNDS; x < (int64_t)g_Map.width + NO_BOUNDS; x++) {
				int out_bounds = x < 0 || y < 0 || x >= (int64_t)g_Map.width || y >= (int64_t)g_Map.height;
				if (out_bounds) continue;
				cell_id = out_bounds ? '\0' : g_Map.data[(size_t)x][(size_t)y];
				int num_neighbors = 0;
				char dominator;
				CalculateSurroundings(x + g_Map.x, y + g_Map.y, &dominator, &num_neighbors);
				if (cell_id != '\0') {
					if (num_neighbors < 2 || num_neighbors > 3) {
						if (num_neighbors > 3 && g_Map.data[(size_t)x][(size_t)y] != (uint8_t)dominator) {
							size_t ind = (int)dominator - 'A';
							g_CapturedCells[ind] += 1;
						}
						g_Map.data[(size_t)x][(size_t)y] += DEATH_MARK;
					}
				} else {
					if (num_neighbors == 3) AddCell(&g_Map, x + g_Map.x, y + g_Map.y, dominator + ('a' - 'A'));
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
		Coordinate coord;
		coord.x = coord.y = 0;
		coord.value = '~';
		EZN_BYTE* added_cells_bytes = calloc(1, 26*sizeof(uint64_t) + sizeof(Coordinate));
		memcpy(added_cells_bytes, &coord, sizeof(Coordinate));
		memcpy(added_cells_bytes + sizeof(Coordinate), g_CapturedCells, 26*sizeof(uint64_t));
		DistributeData((EZN_BYTE*)changed.data, changed.size * sizeof(Coordinate));
		DistributeData(added_cells_bytes, 26*sizeof(uint64_t) + sizeof(Coordinate));
		ARRLIST_Coordinate_clear(&changed);
	} else if (GetNetworkType() == CLIENT) {
		int found_data = TRUE;
		Coordinate coord = { 0 };
		while (found_data) {
			GrabData((EZN_BYTE*)&coord, sizeof(Coordinate), &found_data);
			if (found_data) {
				if (coord.value == '~') {
					GrabValueArray((EZN_BYTE*)g_CapturedCells);
					size_t ind = (size_t)g_ID - 'A';
					g_AvailableCells += g_CapturedCells[ind];
				} else {
					g_Map.data[coord.x][coord.y] = coord.value;
				}
			}
		}
	} else {
		LOG_FATAL("Unknown network type");
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
    g_Camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
	g_Camera.zoom += GetMouseWheelMove() * 0.1f;
	g_Camera.zoom = g_Camera.zoom < 0.1f ? 0.1f : g_Camera.zoom;

	Vector2 currmousepos = GetMousePosition();
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		g_Camera.target.x -= (currmousepos.x - mousepos_old.x) / g_Camera.zoom;
		g_Camera.target.y -= (currmousepos.y - mousepos_old.y) / g_Camera.zoom;
	}
	mousepos_old = currmousepos;

	if (g_Camera.target.x < (-0.5f * CELLSIZE * g_BoardWidth)) g_Camera.target.x = (-0.5f * CELLSIZE * g_BoardWidth);
	if (g_Camera.target.x > (0.5f * CELLSIZE * g_BoardWidth)) g_Camera.target.x = (0.5f * CELLSIZE * g_BoardWidth);
	if (g_Camera.target.y < (-0.5f * CELLSIZE * g_BoardHeight)) g_Camera.target.y = (-0.5f * CELLSIZE * g_BoardHeight);
	if (g_Camera.target.y > (0.5f * CELLSIZE * g_BoardHeight)) g_Camera.target.y = (0.5f * CELLSIZE * g_BoardHeight);
}

void UpdateServer() {
	if (GetNetworkType() == SERVER) {
		size_t datasize;
		char packet_type;
		char* client_data = GrabClientUpdate(&packet_type, &datasize);
		if (client_data != NULL) {
			switch (packet_type) {
				case 'q':
					DynamicCoordinate* coords = (DynamicCoordinate*)client_data;
					for (int i = 0; i < (datasize / sizeof(DynamicCoordinate)); i++) {
						if (GetCell(&g_Map, coords[i].x, coords[i].y) == '\0')
							AddCell(&g_Map, coords[i].x, coords[i].y, coords[i].value);
					}
					break;
				default:
					LOG_WARN("Unknown packet type recieved, unable to properly process client data of type %d - %c", (int)packet_type, packet_type);
			}
			free(client_data);
		}
	}
}

void UpdateLeaderboard() {
	memset(g_Leaderboard, 0, 26*sizeof(uint64_t));
	for (size_t y = 0; y < g_Map.height; y++) {
		for (size_t x = 0; x < g_Map.width; x++) {
			char id = g_Map.data[x][y];
			size_t ind = (size_t)id - 'A';
			g_Leaderboard[ind]++;
		}
	}
}

void UpdateState() {
	uint64_t active_cells = g_Leaderboard[(size_t)(g_ID - 'A')];
	if (active_cells == 0 && g_AvailableCells == 0) {
		g_GameState = GAME_OVER;
	}
}

void CleanCoreScene() {

}

void CalculateSurroundings(int64_t x, int64_t y, char* result, int* count) {
	char ids[9];
	int vals[9];
	int total = 0;
	memset(ids, '\0', 9);
	memset(vals, 0, sizeof(int)*9);
	for (int r = -1; r < 2; r++) {
		for (int c = -1; c < 2; c++) {
			if (r == 0 && c == 0) continue;
			char curr = GetCell(&g_Map, x + c, y + r);
			if (curr != '\0') {
				if ((uint8_t)curr >= (uint8_t)'A' + DEATH_MARK) curr -= DEATH_MARK;
				if (curr >= 'a') continue;
				total += 1;
				int found = -1;
				for (int i = 0; i < 9; i++)
					if (ids[i] == curr) found = i;
				if (found != -1) {
					vals[found] += 1;
				} else {
					for (int i = 0; i < 9; i++) {
						if (ids[i] == '\0') {
							ids[i] = curr;
							vals[i] = 1;
							break;
						}
					}
				}
			}
		}
	}
	size_t max = 0;
	for (int i = 0; i < 9; i++)
		if (vals[i] > vals[max])
			max = i;
	*result = ids[max];
	*count = vals[max];
}

