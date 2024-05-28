#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    const int serverPort = 55554;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(clientSocket);
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        return -1;
    }

    std::string message;
    std::cout << "Enter your message: ";
    std::getline(std::cin, message);

    send(clientSocket, message.c_str(), message.size(), 0);

    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    if (valread > 0) {
        std::cout << "Received: " << std::string(buffer) << std::endl;
    }

    close(clientSocket);
    return 0;
}

