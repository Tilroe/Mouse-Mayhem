#include "app.h"

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <thread>
#include <vector>
#include <string>

// Custom headers
#include "serverConnection.h"
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

App::App() {
	running = false;
}

App::~App() { 
	WSACleanup();
}

void App::start() {
	running = true;
	int result;
	WSADATA wsaData;

	// Start up winsock
	
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		printf("WSAStartup failed with error: %d\n", result);
		return;
	}

	// Connect to server
	ServerConnection server;
	result = server.establishConnection("127.0.0.1");
	if (result != 0) {
		return;
	}

	// main app
	std::vector<std::string> messages;
	messages.push_back("123");
	messages.push_back("456");
	messages.push_back("789");

	char response[DEFAULT_BUFLEN];
	int i = 0;
	while (server.isConnected() && i < 3) {
		//std::this_thread::sleep_for(std::chrono::seconds(5));

		int bytes_sent = server.sendTo(messages[i].c_str(), messages[i].length());
		if (bytes_sent == SOCKET_ERROR) {
			printf("[Client] Failed to send - disconnecting: %d\n", WSAGetLastError());
			break;
		}
		else {
			printf("[Client] Sent: %s\n", messages[i].c_str());
		}
		
		memset(response, 0, DEFAULT_BUFLEN);
		int bytes_recv = server.recvFrom(response, DEFAULT_BUFLEN);
		if (bytes_recv > 0) {
			printf("[Client] Response from server: %s\n", response);
		}
		else if (bytes_recv == 0) {
			printf("[Client] Shutdown request\n");
			break;
		}
		else {
			printf("[Client] Bad recv - disconnecting: %d\n", WSAGetLastError());
		}

		i++;
	}
	server.disconnect();
}