#include "helper.h"

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library headers
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

void socketToIPv4(SOCKET socket, char ip_address[16]) {
    sockaddr name;
    int namelen = sizeof(sockaddr);
    getpeername(socket, &name, &namelen);
    inet_ntop(AF_INET, &(name.sa_data), ip_address, 16);
}

std::vector<std::string> splitString(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}