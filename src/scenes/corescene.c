#include "corescene.h"
#include "core/logger.h"
#include "core/input.h"
#include "utils/timeutils.h"
#include "utils/numbers.h"
#include "data/baked.h"
#include "data/gconfig.h"
#include "data/cellmap.h"
#include "raylib.h"
#include <math.h>

CoreSceneState        g_State              = CORE_NONE;
CoreNetworkObject     g_NetworkObject      = { 0 };
Camera2D              g_Camera             = { 0 };
uint32_t              g_Ping               = -1;
Cellmap               g_Map                = { 0 };

void DrawCoreScene() {
    BeginDrawing();

    ClearBackground(DARKGRAY);
    
	BeginMode2D(g_Camera);

	DrawCells();

	DrawDevObjects();

	EndMode2D();

	DrawDevUI();

    EndDrawing();
}

void DrawCells() {
	Coordinate coord;
	for (size_t i = 0; i < g_Map.active.size; i++) {
		coord = g_Map.active.data[i];
		Rectangle rect = { (float)coord.x * CELLSIZE, (float)coord.y * CELLSIZE, CELLSIZE, CELLSIZE };
		switch(GetCell(&g_Map, coord.x, coord.y)) {
			case 'R':
				DrawRectangleRec(rect, RED);
				break;
			default:
				LOG_WARN("Unintentional cell detected %d, %d", (int)coord.x, (int)coord.y);
				break;
		}
	}
}

void DrawDevObjects() {
	// draw grid
	int stepsize = CELLSIZE;
	int gridsize = 100;
	for (int i = 0; i < gridsize; i++) {
		DrawLine(-1*i*stepsize, -1*gridsize*stepsize, -1*i*stepsize, gridsize*stepsize, LIGHTGRAY);
		DrawLine(i*stepsize, -1*gridsize*stepsize, i*stepsize, gridsize*stepsize, LIGHTGRAY);
		DrawLine(-1*gridsize*stepsize, i*stepsize, gridsize*stepsize, i*stepsize, LIGHTGRAY);
		DrawLine(-1*gridsize*stepsize, -1*i*stepsize, gridsize*stepsize, -1*i*stepsize, LIGHTGRAY);
	}
}

void DrawDevUI() {
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
			#ifndef DEBUG_SINGLEPLAYER
			CheckCoreNetworkService();
			#endif
			MainCoreScene();
			#ifndef DEBUG_SINGLEPLAYER
			UpdateCoreNetworkService();
			#endif
			break;
		default:
			LOG_FATAL("Unhandled core scene state detected");	
	}
}

void InitializeCoreScene() {
	#ifndef DEBUG_SINGLEPLAYER
	// backup initialize connection
	if (g_NetworkObject.m_Descriptor == CORE_IS_UNKNOWN)
		CoreBackupNetworkSetup();
	#endif

	// initialize camera
    g_Camera.target = (Vector2){ 0.0f, 0.0f };
    g_Camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    g_Camera.rotation = 0.0f;
    g_Camera.zoom = 1.0f;

	// erm, what!
	AddCell(&g_Map, 0, 0, 'R');
	AddCell(&g_Map, 9, 10, 'R');
	AddCell(&g_Map, 2, 2, 'R');
	AddCell(&g_Map, -4, 10, 'R');
	AddCell(&g_Map, 1, 1, 'R');
	AddCell(&g_Map, -10, 3, 'R');
	AddCell(&g_Map, -4, -8, 'R');

	// change state
	g_State = CORE_MAIN;
}

void MainCoreScene() {
	// update input modules
	UpdateSmartInput();

	// update camera
	UpdateCoreCamera();
}

void UpdateCoreCamera() {
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
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		g_Camera.target.x -= (currmousepos.x - mousepos_old.x) / g_Camera.zoom;
		g_Camera.target.y -= (currmousepos.y - mousepos_old.y) / g_Camera.zoom;
	}
	mousepos_old = currmousepos;
}

void CoreBackupNetworkSetup() {
	if (g_NetworkObject.m_HostDevice == NULL)
		g_NetworkObject.m_HostDevice = calloc(1, sizeof(ezn_Server));
	if (g_NetworkObject.m_ClientDevice == NULL)
		g_NetworkObject.m_ClientDevice = calloc(1, sizeof(ezn_Client));
	uint8_t address[4];
	ezn_set_ipv4_addr(address, 127, 0, 0, 1);
	if (ezn_configure_client(g_NetworkObject.m_ClientDevice, DEFAULT_CORE_PORT, address) == EZN_ERROR) {
		LOG_FATAL("Unable to set up client configuration");
	} else if (ezn_connect_client(g_NetworkObject.m_ClientDevice, ConnectAsClient) == EZN_ERROR) {
		LOG_WARN("Unable to connect to a local server - starting local server instead");
		if (ezn_generate_server(g_NetworkObject.m_HostDevice, DEFAULT_CORE_PORT) == EZN_ERROR)
			LOG_FATAL("Unable to generate server device");
		if (ezn_open_server(g_NetworkObject.m_HostDevice) == EZN_ERROR)
			LOG_FATAL("Unable to open local server");
		LOG_WARN("Implicitly starting server via backup network setup");
		g_NetworkObject.m_Descriptor = CORE_IS_HOST;
		if (ezn_server_accept(g_NetworkObject.m_HostDevice, HostAsServer, EZN_FALSE) == EZN_ERROR) {
			LOG_FATAL("Unable to accept connections while hosting");
		}
	}
}

void CheckCoreNetworkService() {
	// temp 
	/*
	if (g_NetworkObject.m_ConnectedDevices.size > 0) {
		size_t validation_len;
		EZN_BYTE buff[12];
		if (ezn_ask(ARRLIST_EZN_SOCKET_get(&(g_NetworkObject.m_ConnectedDevices), 0), buff, 12, &validation_len) == EZN_ERROR) {
			LOG_FATAL("error while asking for network packets!");
		}
		if ((int)validation_len > 0) {
			g_EnemyLocation.x = *((float*)buff);
			g_EnemyLocation.y = *((float*)(buff + 4));
			g_Ping = GetUnpreciseEpoch() - *((uint32_t*)(buff + 8));
		}
	} else {
		LOG_FATAL("No server available!");
	}*/

	switch(g_NetworkObject.m_Descriptor) {
		case CORE_IS_CLIENT:
			break;
		case CORE_IS_HOST:
			break;
		default:
			LOG_FATAL("Invalid network status - cannot update network service");
	}
}

void UpdateCoreNetworkService() {
	// temp 
	/*
	if (g_NetworkObject.m_ConnectedDevices.size > 0) {
		size_t validation_len;
		EZN_BYTE buff[12];
		uint32_t currtime = GetUnpreciseEpoch();
		memcpy(buff, &g_PlayerLocation.x, 4);
		memcpy(buff + 4, &g_PlayerLocation.y, 4);
		memcpy(buff + 8, &currtime, 4);
		if (ezn_send(ARRLIST_EZN_SOCKET_get(&(g_NetworkObject.m_ConnectedDevices), 0), buff, 12, &validation_len) == EZN_ERROR) {
			LOG_FATAL("unable to send bytes!");
		}
	} else {
		LOG_FATAL("No server available!");
	}*/

	switch(g_NetworkObject.m_Descriptor) {
		case CORE_IS_CLIENT:
			break;
		case CORE_IS_HOST:
			break;
		default:
			LOG_FATAL("Invalid network status - cannot update network service");
	}
}

void CleanCoreScene() {

}

EZN_STATUS ConnectAsClient(ezn_Client* client, EZN_SOCKET serversock) {
	LOG_WARN("Implicitly connecting as client via backup network setup");
	g_NetworkObject.m_Descriptor = CORE_IS_CLIENT;
	ARRLIST_EZN_SOCKET_add(&(g_NetworkObject.m_ConnectedDevices), serversock);
	return EZN_NONE;
}

EZN_STATUS HostAsServer(ezn_Server* server, EZN_SOCKET clientsock) {
	ARRLIST_EZN_SOCKET_add(&(g_NetworkObject.m_ConnectedDevices), clientsock);
	return EZN_NONE;
}
