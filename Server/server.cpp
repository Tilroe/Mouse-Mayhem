#include "server.h"

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library headers
#include <string>
#include <thread>
#include <mutex>

// Custom headers
#include "consts.h"
#include "clientConnection.h"
#include "helper.h"

#pragma comment(lib, "Ws2_32.lib")

Server::Server() {
    // Start up winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return;
    }
}

Server::~Server() {
	WSACleanup();
}

void Server::start() {
    // Initalize listening socket
    int result = initSocket();
    if (result != 0) {
        return;
    }
    socketToIPv4(ListenSocket, ip_address);

	// Start listening thread and client manager thread
    running = true;
    printf("[Server] Started on %s\n", ip_address);
	std::thread listen_th(&Server::listenForClients, this);

	// Wait till threads finish
	listen_th.join();
}

bool Server::isRunning() { return running; }

int Server::initSocket() {
    int iResult;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // Hints for the type of socket we want
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("[Server] getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("[Server] socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    // Bind the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("[Server] bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return 1;
    }

    freeaddrinfo(result);

    return 0;
}

void Server::listenForClients() {
    // Start listening on socket
    int iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("[Server] listen failed with error: %d\n", WSAGetLastError());
        running = false;
        return;
    }

    char ip_buf[16];

    // Continuously accept clients
    printf("[Server] Listening for clients... \n");
	while (running) {
        SOCKET ClientSocket = INVALID_SOCKET;

        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("[Server] accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        socketToIPv4(ClientSocket, ip_buf);
        printf("[Server] Connected to: %s\n", ip_buf);

        // Add client connection to client pool
        client_pool.create(ClientSocket);
	}

    // Close listening socket
    closesocket(ListenSocket);
}
