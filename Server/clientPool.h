#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

// Custom headers
// #include "clientConnection.h"
class ClientConnection; // forward declaration to avoid circular dependency problems

#pragma comment(lib, "Ws2_32.lib")

class ClientPool {
public:
	ClientPool();
	~ClientPool();
	void create(SOCKET connection);
	std::vector<ClientConnection*> getClients();

private:
	std::vector<ClientConnection*> clients;
	std::mutex clients_lock, stopping_lock;
	std::condition_variable should_stop;
	std::thread prune_th;
	bool running = true;

	void pruneClients();
};