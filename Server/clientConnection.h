#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <string>

// Custom headers
#include "consts.h"
#include "clientPool.h"

#pragma comment(lib, "Ws2_32.lib")

class ClientConnection {
	public:
		ClientConnection(SOCKET socket, ClientPool* pool);
		~ClientConnection();

		void start();
		void disconnect();
		bool isConnected();
		int recvFrom(char* buf, int buflen);
		int sendTo(const char* buf, int buflen);

		std::string getUsername();
		void setUsername(std::string username);
		char* getAddress();

	private:
		SOCKET socket;
		ClientPool *pool;
		char ip_address[16] = "";
		std::string username;
		bool connected = false;
};