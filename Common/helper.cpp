#include "helper.h"

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

void socketToIPv4(SOCKET socket, char ip_address[16]) {
    sockaddr name;
    int namelen = sizeof(sockaddr);
    getpeername(socket, &name, &namelen);
    inet_ntop(AF_INET, &(name.sa_data), ip_address, 16);
}