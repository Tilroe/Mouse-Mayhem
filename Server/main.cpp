// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// Standard library headers
#include <stdio.h>
#include <vector>
#include <string>

// Custom headers
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")

std::vector<HANDLE> threads;
bool running = true;

DWORD WINAPI acceptClients(void* param);
DWORD WINAPI clientThread(void* socket);
DWORD WINAPI threadManager(void* param);

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    HANDLE listenerThread = CreateThread(
        NULL,           // thread attributes
        0,              // thread stack size
        acceptClients,  // thread function
        NULL,           // thread arguments
        0,              // thread creation flags
        NULL            // pointer to thread ID
    );

    HANDLE threadManagerThread = CreateThread(
        NULL,
        0,
        threadManager,
        NULL,
        0,
        NULL
    );

    // Wait for listenerThread to finish (it shouldn't)
    WaitForSingleObject(listenerThread, INFINITE);

    // cleanup
    WSACleanup();

    return 0;
}

DWORD WINAPI acceptClients(void* param) {
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    
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
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("[Server] socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("[Server] bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    }

    // Accept all incoming connections
    printf("Listening for clients...\n");
    while (running) {
        SOCKET ClientSocket = INVALID_SOCKET;

        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("[Server] accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else { 
            sockaddr clientName;
            int namelen = sizeof(sockaddr);
            getpeername(ClientSocket, &clientName, &namelen);
            char ip_address[16];
            inet_ntop(AF_INET, &(clientName.sa_data), ip_address, 16);
            printf("[Server] Connected to: %s\n", ip_address);
        }

        // Start thread for client
        HANDLE clientThreadHandle = CreateThread(
            NULL,                   // thread attributes
            0,                      // thread stack size
            clientThread,           // thread function
            (void *) ClientSocket,  // thread arguments
            0,                      // thread creation flags
            NULL                    // pointer to thread ID
        );
        threads.push_back(clientThreadHandle);
    }

    // Not listening for connections anymore, clean up
    closesocket(ListenSocket);

    return 0;
}

DWORD WINAPI clientThread(void* socket) {
    SOCKET ClientSocket = (SOCKET) socket;

    sockaddr clientName;
    int namelen = sizeof(sockaddr);
    char ip_address_c[16];

    getpeername(ClientSocket, &clientName, &namelen);
    inet_ntop(AF_INET, &(clientName.sa_data), ip_address_c, 16);
    std::string ip_address(ip_address_c);

    printf("Thread created for: %s\n", ip_address.c_str());

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int bytes_recv, bytes_sent;

    bool communicating = true;
    while (communicating) {
        bytes_recv = recv(ClientSocket, recvbuf, recvbuflen, 0);

        // Successful recv
        if (bytes_recv > 0) {
            printf("[%s] Received: %s\n", ip_address.c_str(), recvbuf);
            bytes_sent = send(ClientSocket, recvbuf, bytes_recv, 0); // echo

            // Unsuccessful send
            if (bytes_sent == SOCKET_ERROR) {
                printf("[%s] Bad send - shutting down: %d\n", ip_address.c_str(), WSAGetLastError());
                communicating = false;
            }
            else {
                printf("[%s] Sent: %s\n", ip_address.c_str(), recvbuf);
            }
        }

        // Graceful shutdown
        else if (bytes_recv == 0) {
            printf("[%s] Graceful shutdown\n", ip_address.c_str());
            shutdown(ClientSocket, SD_SEND);
            communicating = false;
        }

        // Recv error
        else {
            printf("[%s] Bad recv - shutting down\n", ip_address.c_str());
            communicating = false;
        }
    }

    closesocket(ClientSocket);
    return 0;
}

DWORD WINAPI threadManager(void* param) {
    int check_interval = 10;

    while (running) {
        // Wait 10 seconds
        Sleep(check_interval * 1000);

        // Erase inactive thread handles
        int n_handles = threads.size();
        int i = 0, deleted_handles = 0;
        while (i < threads.size()) {
            HANDLE thread = threads.at(i);
            if (WaitForSingleObject(thread, 0) == WAIT_OBJECT_0) {
                // Thread is done
                CloseHandle(thread);
                threads.erase(threads.begin() + i);
                deleted_handles++;
            }
            else {
                // Thread is active
                i++;
            }
        }
        printf("[Server] cleaned up %i thread handles\n", deleted_handles);
    }

    return 0;
}