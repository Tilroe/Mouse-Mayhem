#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <string>

// Custom headers
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

class ServerConnection {
public:
	ServerConnection();
	~ServerConnection();
	
	int establishConnection(PCSTR address);
	void disconnect();
	bool isConnected();
	int recvFrom(char* buf, int buflen);
	int sendTo(const char* buf, int buflen);


private:
	SOCKET connection;
	bool connected;
};