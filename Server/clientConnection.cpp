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

#pragma comment(lib, "Ws2_32.lib")

ClientConnection::ClientConnection(SOCKET socket) : socket(socket) {
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

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int bytes_recv, bytes_sent;

	while (connected) {
		memset(recvbuf, 0, DEFAULT_BUFLEN);
		bytes_recv = recvFrom(recvbuf, recvbuflen);

		// Successful recv
		if (bytes_recv > 0) {
			printf("[%s] Received: %s\n", ip_address, recvbuf);
			bytes_sent = sendTo(recvbuf, bytes_recv); // echo

			// Unsuccessful send
			if (bytes_sent == SOCKET_ERROR) {
				printf("[%s] Failed to send - disconnecting: %d\n", ip_address, WSAGetLastError());
				disconnect();
			}
			else {
				printf("[%s] Delivered: %s\n", ip_address, recvbuf);
			}

		}

		// Graceful shutdown
		else if (bytes_recv == 0) {
			printf("[%s] Shutdown request\n", ip_address);
			disconnect();
		}

		// Recv error
		else {
			printf("[%s] Bad recv - disconnecting: %d\n", ip_address, WSAGetLastError());
			disconnect();
		}
	}

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

int ClientConnection::sendTo(char* buf, int buflen) {
	return send(socket, buf, buflen, 0);
}