#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_ATTEMPTS 3
#define BUFFER_SIZE 1024
#define NUM_PORTS 3

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server;
    char message[BUFFER_SIZE] = "Hello";
    char buffer[BUFFER_SIZE] = {0};
    int ports[] = {8888, 8889, 8890};  // Array of ports to try

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("WSAStartup failed");
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket");
        WSACleanup();
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;

    int attempts = 0;
    int connected = 0;

    while (attempts < MAX_ATTEMPTS && !connected) {
        for (int i = 0; i < NUM_PORTS; i++) {
            server.sin_port = htons(ports[i]);

            // Connect to server
            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
                printf("Connection failed on port %d. Retrying...\n", ports[i]);
                continue;
            }

            // Send message
            if (send(sock, message, strlen(message), 0) == SOCKET_ERROR) {
                printf("Send failed");
                closesocket(sock);
                WSACleanup();
                return 1;
            }

            // Receive response
            if (recv(sock, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
                printf("Recv failed");
                break;
            }

            printf("Server: %s\n", buffer);
            connected = 1;
            break;
        }

        attempts++;
        Sleep(1000);  // Sleep for 1 second
    }

    if (!connected) {
        printf("Max attempts reached. Exiting.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}