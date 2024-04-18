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
	CORE_IS_UNKNOWN = 0,
	CORE_IS_HOST,
	CORE_IS_CLIENT
} CoreNetworkDescriptor;

typedef struct {
	CoreNetworkDescriptor m_Descriptor;
	ezn_Server* m_HostDevice;
	ezn_Client* m_ClientDevice;
	ARRLIST_EZN_SOCKET m_ConnectedDevices;
} CoreNetworkObject;

void DrawCoreScene();
void DrawProjectiles();
void DrawDevObjects();
void DrawDevUI();
void UpdateCoreScene();
void UpdateProjectiles();
void InitializeCoreScene();
void MainCoreScene();
void CoreBackupNetworkSetup();
void CheckCoreNetworkService();
void UpdateCoreNetworkService();
void CleanCoreScene();

EZN_STATUS ConnectAsClient(ezn_Client* client, EZN_SOCKET serversock);
EZN_STATUS HostAsServer(ezn_Server* server, EZN_SOCKET clientsock);

#endif
