#include "app.h"

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <string>
#include <iostream>

// Custom headers
#include "serverConnection.h"
#include "helper.h"
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

	std::string input_buf, send_buf, recv_buf;
	char recv_buf_c[DEFAULT_BUFLEN];

	while (server.isConnected()) {
		// Reset buffers
		input_buf.clear();
		send_buf.clear();
		recv_buf.clear();
		memset(recv_buf_c, 0, DEFAULT_BUFLEN);

		// Get user input
		std::cout << "$ ";
		std::getline(std::cin, input_buf);

		// Special input cases
		if (input_buf.empty()) {
			// No input
			continue;
		}
		else if (input_buf.compare("q") == 0)  {
			// Quit
			break;
		}

		// Commands
		std::vector<std::string> tokens = splitString(input_buf, " ");
		std::string command = tokens[0];

		if (command.compare("help") == 0) {
			send_buf = COMM::HELP;
		}
		else if (command.compare("register") == 0) {
			if (tokens.size() == 2) {
				send_buf = send_buf + COMM::REGISTER + ' ' + tokens[1];
			}
			else {
				printf("[Client] correct usage: register [username]\n");
				continue;
			}
		}
		else if (command.compare("list") == 0) {
			if (tokens.size() == 1) {
				send_buf = COMM::LIST_CONNECTIONS;
			}
			else {
				printf("[Client] correct usage: list\n");
				continue;
			}
		}
		else if (command.compare("connect") == 0) {
			if (tokens.size() == 2) {
				send_buf = send_buf + COMM::CONNECT_PEER + ' ' + tokens[1];
			}
			else {
				printf("[Client] correct usage: connect [username]\n");
				continue;
			}
		}
		else {
			printf("[Client] invalid command\n");
			continue;
		}

		// Send input
		int bytes_sent = server.sendTo(send_buf.c_str(), send_buf.size());
		if (bytes_sent == SOCKET_ERROR) {
			printf("[Client] Failed to send to server - disconnecting: %d\n", WSAGetLastError());
			break;
		}
		else if (bytes_sent > 0) {
			printf("[Client] Sent: %s\n", send_buf.c_str());
		}
		
		// Get reponse
		int bytes_recv = server.recvFrom(recv_buf_c, DEFAULT_BUFLEN);
		recv_buf = std::string(recv_buf_c);
		if (bytes_recv > 0) {
			printf("[Client] Response from server:\n");
			printf("%s\n", recv_buf.c_str());
		}
		else if (bytes_recv == 0) {
			printf("[Client] Shutdown request from server\n");
			break;
		}
		else {
			printf("[Client] Bad recv - disconnecting: %d\n", WSAGetLastError());
		}
	}
	server.disconnect();
}