#pragma once

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

void socketToIPv4(SOCKET socket, char ip_address[16]);

std::vector<std::string> splitString(std::string str, std::string delim);