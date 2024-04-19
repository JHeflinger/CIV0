#include "corescene.h"
#include "core/logger.h"
#include "core/input.h"
#include "utils/timeutils.h"
#include "utils/numbers.h"
#include "data/datamap.h"
#include "data/collisionmap.h"
#include "data/baked.h"
#include "data/gconfig.h"
#include "raylib.h"
#include <math.h>

CoreSceneState        g_State              = CORE_NONE;
CoreNetworkObject     g_NetworkObject      = { 0 };
Camera2D              g_Camera             = { 0 };
CollisionMap*         g_CollisionMap       = NULL;
uint32_t              g_Ping               = -1;

void DrawCoreScene() {
    BeginDrawing();

    ClearBackground(DARKGRAY);
    
	BeginMode2D(g_Camera);

	DrawDevObjects();

	EndMode2D();

	DrawDevUI();

    EndDrawing();
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

	// draw collision boxes
	if (g_CollisionMap != NULL) {
		for (int y = 0; y < g_CollisionMap->height; y++) {
			for (int x = 0; x < g_CollisionMap->width; x++) {
				if (g_CollisionMap->data[x][y] == 'B') {
					Rectangle hitbox = {x*CELLSIZE + g_CollisionMap->x*CELLSIZE, y*CELLSIZE + g_CollisionMap->y*CELLSIZE, CELLSIZE, CELLSIZE};
					DrawRectangleRec(hitbox, GREEN);
				}
			}
		}
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

	// initialize collision map 
	Datamap* coldata = GenerateDatamap(TANKS_MAP);
	g_CollisionMap = GenerateCollisionMap();
	LoadCollisionChunk(g_CollisionMap, coldata);
	FreeDatamap(coldata);
	//coldata = GenerateDatamap(DEV_MAP_2);
	//LoadCollisionChunk(g_CollisionMap, coldata);
	//FreeDatamap(coldata);

	// change state
	g_State = CORE_MAIN;
}

void MainCoreScene() {
	// update input modules
	UpdateSmartInput();

	// update velocity
	float ft = GetFrameTime();
	float speed = CELLSIZE * 10;
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

	// update camera
	//g_Camera.target = (Vector2){ g_PlayerLocation.x + g_PlayerSize.x/2.0f, g_PlayerLocation.y + g_PlayerSize.y/2.0f };
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
	FreeCollisionMap(g_CollisionMap);
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
