#include "serverCommands.h"

// Standard library headers
#include <string>

// Custom headers
#include "clientConnection.h"
#include "clientPool.h"
#include "consts.h"

// -- Command Parent Class --
Command::Command(ClientConnection* client) : client(client) {};

// -- Help --
HelpCmd::HelpCmd(ClientConnection* client) : Command(client) {};
void HelpCmd::execute() {
	std::string response;
	response = response + HelpCmd::description + "\n";
	response = response + RegisterCmd::description + "\n";
	response = response + ListConnectionsCmd::description + "\n";
	response = response + ConnectToPeerCmd::description;

	int bytes_sent = client->sendTo(response.c_str(), response.size());
}

// -- Register --
RegisterCmd::RegisterCmd(ClientConnection* client, ClientPool* client_pool, std::string username) : Command(client), client_pool(client_pool), username(username) {};
void RegisterCmd::execute() {
	// TODO :  Check if I need to lock access to clients before

	// Iterate through all connections to see if username is taken
	for (ClientConnection* c : client_pool->getClients()) {
		if (c == client) { continue; } // Skip if looking at self
		if (c->getUsername().compare(username) == 0) {
			// Username is already taken
			int bytes_sent = client->sendTo(COMM::USERNAME_TAKEN, COMM::COMM_LEN);
			return;
		}
	}

	// Username is not taken
	client->setUsername(username);
	client->sendTo(COMM::REGISTRATION_SUCCESSFUL, COMM::COMM_LEN);
}

// -- List Connections --
ListConnectionsCmd::ListConnectionsCmd(ClientConnection* client, ClientPool* client_pool) : Command(client), client_pool(client_pool) {};
void ListConnectionsCmd::execute() {
	std::string response("--- Users ---");

	// Add registered usernames to response
	for (ClientConnection* c : client_pool->getClients()) {
		std::string s = c->getUsername();
		if (!s.empty()) {
			// Non-empty username = registered user
			response = response + '\n' + s;
		}
	}

	client->sendTo(response.c_str(), response.size());
}

// -- Connect To Peer --
ConnectToPeerCmd::ConnectToPeerCmd(ClientConnection* client, ClientPool* client_pool, std::string peer_name) : Command(client), client_pool(client_pool), peer_name(peer_name) {};
void ConnectToPeerCmd::execute() {
	// If not registered, cannot connect to peer
	if (client->getUsername().empty()) { client->sendTo(COMM::NOT_REGISTERED, COMM::COMM_LEN); return; }

	// Cannot connect to self
	if (client->getUsername().compare(peer_name) == 0) { client->sendTo(COMM::INVALID_PEER, COMM::COMM_LEN); return; }

	// Find peer in pool
	for (ClientConnection* c : client_pool->getClients()) {
		if (c == client) { continue; } // Skip if looking at self
		std::string s = c->getUsername();
		if (s.compare(peer_name) == 0) {
			// Peer found
			client->sendTo(c->getAddress(), strlen(c->getAddress()));
			return;
		}
	}

	// Peer not found
	client->sendTo(COMM::PEER_NOT_FOUND, COMM::COMM_LEN);
}