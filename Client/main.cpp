// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library headers

// Custom headers
#include "app.h"

#pragma comment(lib, "Ws2_32.lib")

int main() {
    App app;
    app.start();

    return 0;
}