#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

std::vector<int> clients;
std::mutex clientsMutex;

void handleClientConnection(int clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread > 0) {
            std::string message(buffer);
            std::cout << "Received: " << message << std::endl;

            // Broadcast message to all clients
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (int client : clients) {
                if (client != clientSocket) {
                    send(client, message.c_str(), message.size(), 0);
                }
            }
        } else {
            // Remove client from the list
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            close(clientSocket);
            break;
        }
    }
}

int main() {
    int serverPort = 55554;
    int serverSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverPort);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << serverPort << "..." << std::endl;

    while (true) {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }
        std::thread(handleClientConnection, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}

