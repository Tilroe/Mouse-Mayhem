#include "clientPool.h"

// Windows headers
#include <winsock2.h>

// Standard library headers
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdio.h>

// Custom headers
#include "clientConnection.h"

#pragma comment(lib, "Ws2_32.lib")

ClientPool::ClientPool() {
    // Begin pruning clients
    prune_th = std::thread(&ClientPool::pruneClients, this);
}

ClientPool::~ClientPool() {
    // Notify pruning thread to stop
    {
        std::unique_lock<std::mutex> lock(stopping_lock);
        running = false;
        should_stop.notify_all();
    }
    prune_th.join();

    // Delete all client connections
	for (ClientConnection *connection : clients) {
		delete connection; // Calls destructor
	} 
}

void ClientPool::create(SOCKET connection) {
    // Create new client connection
	ClientConnection* client = new ClientConnection(connection);

    // Start thread for client
    std::thread client_th(&ClientConnection::start, client);
    client_th.detach();

    // Wait for client to start before adding client to list
    while (!client->isConnected());

    // Add client connection to list
	std::lock_guard<std::mutex> lock(clients_lock);
	clients.push_back(client);
}

void ClientPool::pruneClients() {
    printf("[Server] Pruning clients\n");
	while (running) {
        // Wait 10 seconds, or until notified to stop by constructor
        {
            std::unique_lock<std::mutex> lock(stopping_lock);
            if (should_stop.wait_for(lock, std::chrono::seconds(10), [this] { return !running; })) {
                break;
            }
        }

        // Lock access to clients
        std::lock_guard<std::mutex> lock(clients_lock);

        // Erase inactive clients
        int n_inactive_clients = 0, i = 0;
        while (i < clients.size()) {
            if (clients.at(i)->isConnected()) {
                i++;
            }
            else {
                // Free dynamic memory first, then remove from vector
                delete clients.at(i);
                clients.erase(clients.begin() + i);
                n_inactive_clients++;
            }
        }

        printf("[Server] Released %i inactive clients\n", n_inactive_clients);
	}
    printf("[Server] Finished pruning clients\n");
}