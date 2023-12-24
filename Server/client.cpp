#include "client.h"

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

Client::Client(SOCKET socket) {
	this->socket = socket;
	active = false;
	socketToIPv4(this->socket, ip_address);
	printf("[%s] Created\n", ip_address);
}

Client::~Client() {
	active = false;
	closesocket(socket);
}

void Client::start() {
	printf("[%s] [tid = %d] Started\n", ip_address, std::this_thread::get_id());
	active = true;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int bytes_recv, bytes_sent;

	while (active) {
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

void Client::disconnect() {
	active = false;
	shutdown(socket, SD_SEND);
	printf("[%s] Shutdown\n", ip_address);
}

bool Client::isActive() {
	return active;
}

int Client::recvFrom(char* buf, int buflen) {
	return recv(socket, buf, buflen, 0);
}

int Client::sendTo(char* buf, int buflen) {
	return send(socket, buf, buflen, 0);
}