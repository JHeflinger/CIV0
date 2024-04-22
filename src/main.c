#include "core/game.h"
#include "network/network.h"
#include "core/logger.h"

int main(int argc, char** argv) {
	if (argc != 1 && argc != 2) {
		LOG_FATAL("Wrong number of arguments! Please use 0 arguments or specify a port to host a server on.");
	} else {
		trash_func(argc == 1 ? SERVER : CLIENT);
	}
    RunGame();
    return 0;
}
