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
    result = initSocket();
    if (result != 0) {
        return;
    }
    socketToIPv4(ListenSocket, ip_address);

	// Start listening thread and client manager thread
    running = true;
    printf("[Server] Started on %s\n", ip_address);
	std::thread listen_th(&Server::listenForClients, this);
	std::thread manage_clients_th(&Server::releaseClients, this);

	// Wait till threads finish
	listen_th.join();
	manage_clients_th.join();
}

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
    printf("[Server] [tid: %d] Listening for clients... \n", std::this_thread::get_id());
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
        ClientConnection *client = new ClientConnection(ClientSocket);

        // Start thread for client
        std::thread client_th(&ClientConnection::start, client);
        client_th.detach();

        // Wait till client is running before adding it to list
        while (!client->isActive());
        addClient(client);
	}

    // Close listening socket
    closesocket(ListenSocket);
}

void Server::releaseClients() {
    printf("[Server] [tid: %d] Cleaning up inactive clients... \n", std::this_thread::get_id());

	while (running) {
        // Wait 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Lock access to clients
        std::lock_guard<std::mutex> lock(client_list_lock);

        // Erase inactive clients
        int n_inactive_clients = 0, i = 0;
        while (i < clients.size()) {
            if (clients.at(i)->isActive()) {
                i++;
            }
            else {
                // Free dynamic memory first, then remove from vector
                delete clients.at(i);
                clients.erase(clients.begin() + i);
                n_inactive_clients++;
            }
        }

        printf("[Server] Released %i inactive clients\n", n_inactive_clients);
	}
}

void Server::addClient(ClientConnection *client) {
    std::lock_guard<std::mutex> lock(client_list_lock);
    clients.push_back(client);
}