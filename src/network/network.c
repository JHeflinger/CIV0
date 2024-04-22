#include "network.h"
#include "core/logger.h"
#include "data/gconfig.h"
#include "easynet.h"

ezn_Server  g_Server = { 0 };
ezn_Client  g_Client = { 0 };
NetworkType g_Type   = 0;

int InitializeNetwork(NetworkType type) {
	ezn_init();
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
