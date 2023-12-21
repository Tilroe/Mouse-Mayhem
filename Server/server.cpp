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
#include "client.h"
#include "helper.h"

#pragma comment(lib, "Ws2_32.lib")

Server::Server() {
	running = false;
    ListenSocket = INVALID_SOCKET;
}

Server::~Server() {
	WSACleanup();
}

void Server::start() {
	// Start up winsock
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		printf("WSAStartup failed with error: %d\n", result);
		return;
	}

    // Initalize listening socket
    initSocket();
    socketToIPv4(ListenSocket, ip_address);

	// Start listening thread and client manager thread
    running = true;
	std::thread listen_th(&Server::listenForClients, this);
	std::thread manage_clients_th(&Server::releaseClients, this);

	// Wait till threads finish
	listen_th.join();
	manage_clients_th.join();
}

void Server::initSocket() {
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
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("[Server] getaddrinfo failed with error: %d\n", iResult);
        return;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("[Server] socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return;
    }

    // Bind the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("[Server] bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return;
    }

    freeaddrinfo(result);
}

void Server::listenForClients() {
    // Start listening on socket
    int iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("[Server] listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        return;
    }

    char ip_buf[16];

    // Continuously accept clients
    printf("[Server] Listening for clients...\n");
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

        // Create new client
        Client client(ClientSocket);
        addClient(client);

        // Start thread for client
        std::thread client_th(&Client::start, &client);
	}

    // Close listening socket
    closesocket(ListenSocket);
}

void Server::releaseClients() {
	while (running) {
        // Wait 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Lock access to clients
        std::lock_guard<std::mutex> lock(client_list_lock);

        // Erase inactive clients
        int n_inactive_clients = 0, i = 0;
        while (i < clients.size()) {
            if (clients.at(i).isActive()) {
                i++;
            }
            else {
                clients.erase(clients.begin() + i);
                n_inactive_clients++;
            }
        }

        printf("[Server] Released %i inactive clients\n", n_inactive_clients);
	}
}

void Server::addClient(Client& client) {
    std::lock_guard<std::mutex> lock(client_list_lock);
    clients.push_back(client);
}