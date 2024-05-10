#ifndef NETWORK_H
#define NETWORK_H

#include "easynet.h"

typedef enum {
	SERVER,
	CLIENT
} NetworkType;

void StartServer();
char ConnectClient();
void DistributeData(EZN_BYTE* data, size_t size);
void GrabData(EZN_BYTE* data, size_t size, int* found_data);
void GrabValueArray(EZN_BYTE* data);
void SendPacket(char packet_header, EZN_BYTE* data, size_t size);
char* GrabClientUpdate(char* packet_type, size_t* datasize);
int InitializeNetwork(NetworkType type);
int ShutdownNetwork();
NetworkType GetNetworkType();

void ServerAcceptBehavior(void* params);

EZN_STATUS AttatchClient(ezn_Server* server, EZN_SOCKET clientsock);
EZN_STATUS RegisterServer(ezn_Client* client, EZN_SOCKET serversock);

#endif
