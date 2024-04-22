#ifndef NETWORK_H
#define NETWORK_H

typedef enum {
	SERVER,
	CLIENT
} NetworkType;

int InitializeNetwork(NetworkType type);
int ShutdownNetwork();
NetworkType GetNetworkType();

#endif
