#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <mutex>

// Custom headers
#include "consts.h"
#include "client.h"

#pragma comment(lib, "Ws2_32.lib")

class Server {
public: 
	Server();
	~Server();
	void start();

private:
	SOCKET ListenSocket;
	char ip_address[16]{""};

	std::vector<Client*> clients;
	std::mutex client_list_lock;
	bool running;

	int initSocket();
	void listenForClients();
	void releaseClients();
	void addClient(Client *client);
};