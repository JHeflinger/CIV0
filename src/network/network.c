#include "network.h"
#include "core/logger.h"
#include "data/gconfig.h"
#include "utils/datastructs.h"

ezn_Server         g_Server = { 0 }; // the server that is being hosted
ezn_Client         g_Client = { 0 }; // the client that is being implemented
NetworkType        g_Type   = 0;
EZN_MUTEX          g_Mutex;
EZN_THREAD         g_ServerAcceptThread;
ARRLIST_EZN_SOCKET g_ConnectedClients; // the sockets of the clients connected to the hosted server
EZN_SOCKET         g_ServerConnection; // socket of the server that the client is connected to

void StartServer() {
	EZN_CREATE_THREAD(g_ServerAcceptThread, ServerAcceptBehavior, NULL);
}

void ConnectClient() {
	if (ezn_connect_client(&g_Client, RegisterServer) == EZN_ERROR) LOG_FATAL("Unable to connect client service");
}

void DistributeData(EZN_BYTE* data, size_t size) {
	size_t sent = 0;
	for (size_t i = 0; i < g_ConnectedClients.size; i++) {
		if (ezn_send(g_ConnectedClients.data[i], data, size, &sent) == EZN_ERROR || sent != size) {
			LOG_WARN("Unable to send data properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)size);
		}
	}
}

void GrabData(EZN_BYTE* data, size_t size, int* found_data) {
	size_t grabbed = 0;
	if (ezn_ask(g_ServerConnection, data, size, &grabbed) == EZN_ERROR) LOG_WARN("Unable to properly read data from server");
	*found_data = grabbed > 0 && grabbed <= size ? TRUE : FALSE;
}

int InitializeNetwork(NetworkType type) {
	ezn_init();
	EZN_CREATE_MUTEX(g_Mutex);
	g_Type = type;
	if (type == SERVER) {
		if (ezn_generate_server(&g_Server, FALLBACK_PORT) == EZN_ERROR) return FALSE;
		if (ezn_open_server(&g_Server) == EZN_ERROR) return FALSE;
	} else if (type == CLIENT) {
		uint8_t address[IPV4_ADDR_LENGTH];
		ezn_str_to_ipaddr(address, FALLBACK_IP);
		if (ezn_configure_client(&g_Client, FALLBACK_PORT, address) == EZN_ERROR) return FALSE;
	} else {
		LOG_FATAL("Unknown network type detected");
	}
	return TRUE;
}

int ShutdownNetwork() {
	if (g_Type == SERVER && ezn_close_server(&g_Server) == EZN_ERROR) return FALSE;
	if (ezn_safe_clean() == EZN_ERROR) return FALSE;
	ezn_clean();
	return TRUE;
}

NetworkType GetNetworkType() {
	return g_Type;
}

void ServerAcceptBehavior(void* params) {
	while (TRUE) {
		if (ezn_server_accept(&g_Server, AttatchClient, EZN_FALSE) == EZN_ERROR) {
			LOG_FATAL("an error occured while attempting to accept a client connection");
		}
	}
}

EZN_STATUS AttatchClient(ezn_Server* server, EZN_SOCKET clientsock) {
	ARRLIST_EZN_SOCKET_add(&g_ConnectedClients, clientsock);
	return EZN_NONE;
}

EZN_STATUS RegisterServer(ezn_Client* client, EZN_SOCKET serversock) {
	g_ServerConnection = serversock;
	return EZN_NONE;
}