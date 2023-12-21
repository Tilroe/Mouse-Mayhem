#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <string>

// Custom headers
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

class Client {
	public:
		Client(SOCKET socket);
		~Client();
		void start();
		void disconnect();
		bool isActive();
		int recvFrom(char* buf, int buflen);
		int sendTo(char* buf, int buflen);
		

	private:
		SOCKET socket;
		char ip_address[16];
		std::string username;
		bool active;
};