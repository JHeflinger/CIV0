#include "network.h"
#include "core/logger.h"
#include "data/gconfig.h"
#include "network/teams.h"
#include "utils/datastructs.h"

ezn_Server         g_Server = { 0 }; // the server that is being hosted
ezn_Client         g_Client = { 0 }; // the client that is being implemented
NetworkType        g_Type   = 0;
EZN_MUTEX          g_Mutex;
EZN_THREAD         g_ServerAcceptThread;
ARRLIST_EZN_SOCKET g_ConnectedClients; // the sockets of the clients connected to the hosted server
EZN_SOCKET         g_ServerConnection; // socket of the server that the client is connected to
uint8_t            g_ip[4];
uint16_t           g_port;    
int                g_configured = FALSE;
int                g_BoardWidth;
int                g_BoardHeight;
int                g_Initialized = FALSE;

void StartServer() {
	if (g_Initialized == FALSE) InitializeNetwork(SERVER);
	EZN_CREATE_THREAD(g_ServerAcceptThread, ServerAcceptBehavior, NULL);
}

char ConnectClient() {
	if (ezn_connect_client(&g_Client, RegisterServer) == EZN_ERROR) LOG_FATAL("Unable to connect client service");
	return GetRotatingID();
}

void DistributeData(EZN_BYTE* data, size_t size) {
	size_t sent = 0;
	for (size_t i = 0; i < g_ConnectedClients.size; i++) {
		if (ezn_send(g_ConnectedClients.data[i], data, size, &sent) == EZN_ERROR || sent != size) {
			LOG_WARN("Unable to send data properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)size);
			LOG_WARN("Disconnecting client");
			ARRLIST_EZN_SOCKET_remove(&g_ConnectedClients, i);
			i--;
		}
	}
}

void GrabData(EZN_BYTE* data, size_t size, int* found_data) {
	size_t grabbed = 0;
	if (ezn_ask(g_ServerConnection, data, size, &grabbed) == EZN_ERROR) LOG_WARN("Unable to properly read data from server");
	*found_data = grabbed > 0 && grabbed <= size ? TRUE : FALSE;
}

void GrabValueArray(EZN_BYTE* data) {
	size_t grabbed = 0;
	if (ezn_recieve(g_ServerConnection, data, 26*sizeof(uint64_t), &grabbed) == EZN_ERROR || grabbed != 26*sizeof(uint64_t)) LOG_WARN("Unable to properly grab network value array");
}

void SendPacket(char packet_header, EZN_BYTE* data, size_t size) {
	size_t sent = 0;
	uint32_t packsize = size;
	if (ezn_send(g_ServerConnection, (EZN_BYTE*)&packet_header, sizeof(char), &sent) == EZN_ERROR || sent != sizeof(char))
		LOG_WARN("Unable to send data to server properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)sizeof(char));
	if (ezn_send(g_ServerConnection, (EZN_BYTE*)&packsize, sizeof(uint32_t), &sent) == EZN_ERROR || sent != sizeof(uint32_t))
		LOG_WARN("Unable to send data to server properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)sizeof(uint32_t));
	if (ezn_send(g_ServerConnection, data, size, &sent) == EZN_ERROR || sent != size)
		LOG_WARN("Unable to send data to server properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)size);
}

char* GrabClientUpdate(char* packet_type, size_t* datasize) {
	for (size_t i = 0; i < g_ConnectedClients.size; i++) {
		size_t data_rec;
		if (ezn_ask(g_ConnectedClients.data[i], (EZN_BYTE*)packet_type, sizeof(char), &data_rec) == EZN_ERROR) {
			LOG_FATAL("Network was unable to properly recieve client data");
		}
		if (data_rec == sizeof(char)) {
			uint32_t size;
			if (ezn_recieve(g_ConnectedClients.data[i], (EZN_BYTE*)&size, sizeof(uint32_t), &data_rec) == EZN_ERROR || data_rec != sizeof(uint32_t)) {
				LOG_FATAL("Network was unable to properly recieve client data");
			}
			*datasize = (size_t)size;
			char* data = calloc((size_t)size, sizeof(char));
			if (ezn_recieve(g_ConnectedClients.data[i], (EZN_BYTE*)data, (size_t)size, &data_rec) == EZN_ERROR || data_rec != (size_t)size) {
				LOG_FATAL("Network was unable to properly recieve client data");
			}
			return data;
		}
	}
	return NULL;
}

int InitializeNetwork(NetworkType type) {
	ezn_init();
	EZN_CREATE_MUTEX(g_Mutex);
	g_Type = type;
	if (type == SERVER) {
		if (ezn_generate_server(&g_Server, g_configured ? g_port : FALLBACK_PORT) == EZN_ERROR) return FALSE;
		if (ezn_open_server(&g_Server) == EZN_ERROR) return FALSE;
	} else if (type == CLIENT) {
		uint8_t address[IPV4_ADDR_LENGTH];
		ezn_str_to_ipaddr(address, FALLBACK_IP);
		if (ezn_configure_client(&g_Client, g_configured ? g_port : FALLBACK_PORT, g_configured ? g_ip : address) == EZN_ERROR) return FALSE;
	} else {
		LOG_FATAL("Unknown network type detected");
	}
	g_Initialized = TRUE;
	return TRUE;
}

int ShutdownNetwork() {
	if (g_Initialized) {
		if (g_Type == SERVER && ezn_close_server(&g_Server) == EZN_ERROR) return FALSE;
		if (ezn_safe_clean() == EZN_ERROR) return FALSE;
		ezn_clean();
	}
	g_Initialized = FALSE;
	return TRUE;
}

NetworkType GetNetworkType() {
	return g_Type;
}

void ConfigureNetwork(uint8_t* ip, uint16_t port) {
	g_configured = TRUE;
	memcpy(g_ip, ip, sizeof(uint8_t)*4);
	g_port = port;
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
	Coordinate board_setup;
	board_setup.value = '@';
	board_setup.x = g_BoardWidth;
	board_setup.y = g_BoardHeight;
	size_t sent;
	if (ezn_send(clientsock, (EZN_BYTE*)&board_setup, sizeof(Coordinate), &sent) == EZN_ERROR || sent != sizeof(Coordinate)) {
		LOG_WARN("Unable to send data properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)sizeof(Coordinate));
	}
	Coordinate id_setup;
	id_setup.value = '#';
	id_setup.x = (size_t)(GetRotatingID() - 'A');
	id_setup.y = 0;
	if (ezn_send(clientsock, (EZN_BYTE*)&id_setup, sizeof(Coordinate), &sent) == EZN_ERROR || sent != sizeof(Coordinate)) {
		LOG_WARN("Unable to send data properly - only able to send %lu/%lu bytes", (unsigned long)sent, (unsigned long)sizeof(Coordinate));
	}
	return EZN_NONE;
}

EZN_STATUS RegisterServer(ezn_Client* client, EZN_SOCKET serversock) {
	g_ServerConnection = serversock;
	return EZN_NONE;
}

void ConfigureBoard(int width, int height) {
	g_BoardWidth = width;
	g_BoardHeight = height;
}
