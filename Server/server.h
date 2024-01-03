#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers

// Custom headers
#include "consts.h"
#include "clientConnection.h"
#include "clientPool.h"

#pragma comment(lib, "Ws2_32.lib")

class Server {
public: 
	Server();
	~Server();
	void start();
	bool isRunning();

private:
	SOCKET ListenSocket = INVALID_SOCKET;
	ClientPool client_pool;
	char ip_address[16] = "";
	bool running = false;

	int initSocket();
	void listenForClients();
};