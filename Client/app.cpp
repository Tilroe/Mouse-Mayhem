#include "app.h"

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <string>
#include <iostream>

// Custom headers
#include "serverConnection.h"
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

App::App() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		printf("WSAStartup failed with error: %d\n", result);
	}
	running = false;
}

App::~App() { 
	WSACleanup();
}

void App::start() {
	running = true;

	// Connect to server
	ServerConnection server;
	int result = server.establishConnection("127.0.0.1");
	if (result != 0) {
		return;
	}

	// main app

	std::string send_buf, recv_buf;
	char recv_buf_c[DEFAULT_BUFLEN];

	while (server.isConnected()) {
		// Reset buffers
		send_buf.clear();
		recv_buf.clear();
		memset(recv_buf_c, 0, DEFAULT_BUFLEN);

		// Get user input
		std::cout << "$ ";
		std::getline(std::cin, send_buf);

		// Special input cases
		if (send_buf.empty()) {
			// No input
			continue;
		}
		else if (send_buf.compare("q") == 0)  {
			// Quit
			break;
		}

		// Send input
		int bytes_sent = server.sendTo(send_buf.c_str(), send_buf.size());
		if (bytes_sent == SOCKET_ERROR) {
			printf("[Client] Failed to send - disconnecting: %d\n", WSAGetLastError());
			break;
		}
		else {
			printf("[Client] Sent: %s\n", send_buf.c_str());
		}
		
		// Get reponse
		int bytes_recv = server.recvFrom(recv_buf_c, DEFAULT_BUFLEN);
		recv_buf = std::string(recv_buf_c);
		if (bytes_recv > 0) {
			printf("[Client] Response from server: %s\n", recv_buf.c_str());
		}
		else if (bytes_recv == 0) {
			printf("[Client] Shutdown request\n");
			break;
		}
		else {
			printf("[Client] Bad recv - disconnecting: %d\n", WSAGetLastError());
		}
	}
	server.disconnect();
}