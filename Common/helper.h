#pragma once

// Windows headers
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

void socketToIPv4(SOCKET socket, char ip_address[16]);