#include "clientConnection.h"

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library headers
#include <string>
#include <thread>

// Custom headers
#include "consts.h"
#include "helper.h"
#include "serverCommands.h"

#pragma comment(lib, "Ws2_32.lib")

ClientConnection::ClientConnection(SOCKET socket, ClientPool* pool) : socket(socket), pool(pool) {
	socketToIPv4(this->socket, ip_address);
	printf("[%s] Created\n", ip_address);
}

ClientConnection::~ClientConnection() {
	if (connected) { disconnect(); }
	closesocket(socket);
}

void ClientConnection::start() {
	printf("[%s] Started\n", ip_address);
	connected = true;

	std::string send_buf, recv_buf;
	char recv_buf_c[DEFAULT_BUFLEN];
	int bytes_recv;

	while (connected) {
		// Reset buffers
		send_buf.clear();
		recv_buf.clear();
		memset(recv_buf_c, 0, DEFAULT_BUFLEN);

		// Get command from client
		bytes_recv = recvFrom(recv_buf_c, DEFAULT_BUFLEN);

		// Successful recv
		if (bytes_recv > 0) {
			printf("[%s] Received: %s\n", ip_address, recv_buf_c);

			// Resolve command
			recv_buf = std::string(recv_buf_c);
			std::vector<std::string> tokens = splitString(recv_buf, " ");
			std::string command = tokens[0];
			Command* cmd;

			if (command.compare(COMM::HELP) == 0) {
				cmd = new HelpCmd(this);
			}
			else if (command.compare(COMM::REGISTER) == 0) {
				cmd = new RegisterCmd(this, pool, tokens[1]);
			}
			else if (command.compare(COMM::LIST_CONNECTIONS) == 0) {
				cmd = new ListConnectionsCmd(this, pool);
			}
			else if (command.compare(COMM::CONNECT_PEER) == 0) {
				cmd = new ConnectToPeerCmd(this, pool, tokens[1]);
			}
			else {
				printf("[%s] Bad command - disconnecting\n", ip_address);
				break;
			}

			// Execute command
			cmd->execute();
			delete cmd;
		}

		// Graceful shutdown
		else if (bytes_recv == 0) {
			printf("[%s] Shutdown request\n", ip_address);
			break;
		}

		// Recv error
		else {
			printf("[%s] Bad recv - disconnecting: %d\n", ip_address, WSAGetLastError());
			break;
		}
	}
	disconnect();
	return;
}

void ClientConnection::disconnect() {
	connected = false;
	shutdown(socket, SD_BOTH);
	printf("[%s] Shutdown\n", ip_address);
}

bool ClientConnection::isConnected() {
	return connected;
}

int ClientConnection::recvFrom(char* buf, int buflen) {
	return recv(socket, buf, buflen, 0);
}

int ClientConnection::sendTo(const char* buf, int buflen) {
	return send(socket, buf, buflen, 0);
}

std::string ClientConnection::getUsername() { return username; }

void ClientConnection::setUsername(std::string username) { this->username = username; }

char* ClientConnection::getAddress() { return ip_address; }