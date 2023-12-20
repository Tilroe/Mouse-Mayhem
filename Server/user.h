#pragma once

#include <winsock2.h>
#include <string>
#include "consts.h"

#pragma comment(lib, "Ws2_32.lib")


class User {
	public:
		User(std::string name, addrinfo address);
		std::string getName();
		addrinfo getAddress();

	private:
		std::string name;
		addrinfo address;
};