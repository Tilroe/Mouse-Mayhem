#pragma once

// Standard library headers
#include <string>

// Custom headers
#include "clientConnection.h"
#include "clientPool.h"

class Command {
public:
	Command(ClientConnection* client);
	virtual void execute() = 0; // pure virtual, MUST be overriden
	static constexpr const char* description = "";
protected:
	ClientConnection* client;
};

class HelpCmd : public Command {
public:
	HelpCmd(ClientConnection* client);
	void execute() override;
	static constexpr const char* description = "help : list all commands";
};

class RegisterCmd : public Command {
public:
	RegisterCmd(ClientConnection* client, ClientPool* client_pool, std::string username);
	void execute() override;
	static constexpr const char* description = "register [username] : register your username with server";
private:
	ClientPool* client_pool;
	std::string username;
};

class ListConnectionsCmd : public Command {
public:
	ListConnectionsCmd(ClientConnection* client, ClientPool* client_pool);
	void execute() override;
	static constexpr const char* description = "list : list all registered connections";
private:
	ClientPool* client_pool;
};

class ConnectToPeerCmd : public Command {
public:
	ConnectToPeerCmd(ClientConnection* client, ClientPool* client_pool, std::string peer_name);
	void execute() override;
	static constexpr const char* description = "connect [username]: connects to peer with given username";
private:
	ClientPool* client_pool;
	std::string peer_name;
};