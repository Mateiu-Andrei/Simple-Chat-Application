#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <algorithm>

std::vector<int> clientSockets; // store client sockets
std::mutex clientsMutex; // Syncronization for the socket vector

void handleClientConnection(int clientSocket, struct sockaddr_in address, int serverPort) {
    char buffer[1024] = {0}; // store incoming messages
    while (true) {
        int valread = read(clientSocket, buffer, 1024); 
        if (valread <= 0) { // error handling
            close(clientSocket);
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
            return;
        }

        std::string message(buffer); 
        if (message.find("/exit") != std::string::npos) { // exit message check
            close(clientSocket);
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
            return;
        }

        std::string formattedMessage = "\n" + message; 

        // lock the mutex and send the message to all clients except the sender
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

    // create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    // set socket options
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // IP address
    address.sin_port = htons(serverPort); // set port number

    // bind the socket to the network address and port
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Listen");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << serverPort << "..." << std::endl;

    std::vector<std::thread> threads; // store threads

    while (true) {
        int clientSocket;
        // accept incoming connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            continue;
        }

        // add new client to the list
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        // start a new thread for the new client
        threads.emplace_back(handleClientConnection, clientSocket, address, serverPort);
    }

    
    
    close(serverSocket); 
    return 0;
}

