#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <windows.h>

#include "consts.h"
#include "user.h"

#pragma comment(lib, "Ws2_32.lib")

std::map<User, HANDLE> users;

DWORD WINAPI acceptClients(void* param);
DWORD WINAPI clientThread(void* socket);

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
        printf("getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
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
    bool acceptingConnections = true;
    printf("Listening for clients...\n");
    while (acceptingConnections) {
        SOCKET ClientSocket = INVALID_SOCKET;

        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
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
            printf("Connected to: %s\n", ip_address);
        }

        // Start thread for client
        HANDLE clientThreadHandle = CreateThread(
            NULL,                   // thread attributes
            0,                      // thread stack size
            clientThread,           // thread function
            (void *) ClientSocket,   // thread arguments
            0,                      // thread creation flags
            NULL                    // pointer to thread ID
        );
    }

    // Not listening for connections anymore, close listening socket
    closesocket(ListenSocket);

    return 0;
}

DWORD WINAPI clientThread(void* socket) {
    SOCKET ClientSocket = (SOCKET) socket;

    sockaddr clientName;
    int namelen = sizeof(sockaddr);
    getpeername(ClientSocket, &clientName, &namelen);
    char ip_address[16];
    inet_ntop(AF_INET, &(clientName.sa_data), ip_address, 16);
    printf("Thread created for: %s\n", ip_address);


    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int bytes_recv, bytes_sent;

    bool communicating = true;
    while (communicating) {
        bytes_recv = recv(ClientSocket, recvbuf, recvbuflen, 0);

        // Successful recv
        if (bytes_recv > 0) {
            printf("received: %s\n", recvbuf);
            bytes_sent = send(ClientSocket, recvbuf, bytes_recv, 0); // echo

            // Unsuccessful send
            if (bytes_sent == SOCKET_ERROR) {
                communicating = false;
            }
        }

        // Graceful shutdown
        else if (bytes_recv == 0) {
            shutdown(ClientSocket, SD_SEND);
            communicating = false;
        }

        // Recv error
        else {
            communicating = false;
        }
    }

    closesocket(ClientSocket);
    return 0;
}