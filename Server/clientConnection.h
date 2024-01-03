#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <string>

// Custom headers
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

class ClientConnection {
	public:
		ClientConnection(SOCKET socket);
		~ClientConnection();
		void start();
		void disconnect();
		bool isConnected();
		int recvFrom(char* buf, int buflen);
		int sendTo(char* buf, int buflen);
		

	private:
		SOCKET socket;
		char ip_address[16] = "";
		std::string username;
		bool connected = false;
};