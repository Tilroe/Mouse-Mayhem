#include "serverConnection.h"

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library headers
#include <thread>

// Custom headers
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

ServerConnection::ServerConnection() {
	connection = INVALID_SOCKET;
	connected = false;
}

ServerConnection::~ServerConnection() {
    if (connected) { disconnect(); }
	closesocket(connection);
}

int ServerConnection::establishConnection(PCSTR address) {
    struct addrinfo *result = NULL, hints;
    int iResult;

    // Hints for the type of socket we want
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(address, DEFAULT_PORT, &hints, &result); // null means server is on same host as client
    if (iResult != 0) {
        printf("[Client] getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }

    // Create socket
    connection = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connection == INVALID_SOCKET) {
        printf("[Client] socket creation failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    // Attempt to connect
    iResult = connect(connection, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("[Client] Failed to connect to server with error: %d\n", WSAGetLastError());
        return 1;
    }

    freeaddrinfo(result); // frees dynamic memory from getaddrinfo

    connected = true;
    printf("[Client] Connection to server established\n");
    return 0;
}

void ServerConnection::disconnect() {
	if (!(connection == INVALID_SOCKET)) {
		shutdown(connection, SD_SEND);
	}
	connected = false;
    printf("[Client] Disconnected from server\n");
}

bool ServerConnection::isConnected() {
	return connected;
}

int ServerConnection::recvFrom(char* buf, int buflen) {
	return recv(connection, buf, buflen, 0);
}

int ServerConnection::sendTo(const char* buf, int buflen) {
	return send(connection, buf, buflen, 0);
}