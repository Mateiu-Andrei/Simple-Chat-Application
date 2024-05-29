#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <algorithm>

std::vector<int> clientSockets;
std::mutex clientsMutex;

void handleClientConnection(int clientSocket, struct sockaddr_in address, int serverPort) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread <= 0) {
            close(clientSocket);
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
            return;
        }

        std::string message(buffer);
        if (message.find("/exit") != std::string::npos) {
            close(clientSocket);
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
            return;
        }

        std::string formattedMessage = "\n" + message; // Add newline before the message

        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int socket : clientSockets) {
            if (socket != clientSocket) {
                send(socket, formattedMessage.c_str(), formattedMessage.size(), 0);
            }
        }
        memset(buffer, 0, sizeof(buffer));
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

    std::vector<std::thread> threads;

    while (true) {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        threads.emplace_back(handleClientConnection, clientSocket, address, serverPort);
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    close(serverSocket);
    return 0;
}

