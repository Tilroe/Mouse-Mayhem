#pragma once

#include "serverConnection.h"

class App {
	public:
		App();
		~App();
		void start();

	private:
		bool running;
		ServerConnection server;
};