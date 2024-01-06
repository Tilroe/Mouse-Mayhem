#pragma once

constexpr auto DEFAULT_PORT = "1234";
constexpr auto DEFAULT_BUFLEN = 512;

namespace COMM {
	const int COMM_LEN = 3;

	// Common
	const char NOT_REGISTERED[COMM_LEN]				= "NR";

	// Help
	const char HELP[COMM_LEN]						= "HP";

	// Registration 
	const char REGISTER[COMM_LEN]					= "RG";
	const char USERNAME_TAKEN[COMM_LEN]				= "UT";
	const char REGISTRATION_SUCCESSFUL[COMM_LEN]	= "SR";

	// List Connections
	const char LIST_CONNECTIONS[COMM_LEN]			= "LC";

	// Connect To Peer
	const char CONNECT_PEER[COMM_LEN]				= "CP";
	const char INVALID_PEER[COMM_LEN]				= "IP";
	const char PEER_NOT_FOUND[COMM_LEN]				= "PN";

}