#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

void clearConsole() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the console
}

void receiveMessages(int clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread <= 0) {
            std::cout << "Disconnected from server" << std::endl;
            close(clientSocket);
            exit(0);
        }
        std::string message(buffer);
        std::cout << "\n" << message << std::endl;
        std::cout << "> "; // Reprint the prompt
        fflush(stdout); // Ensure the prompt is displayed immediately
        memset(buffer, 0, sizeof(buffer));
    }
}

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

    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    clearConsole();
    
    std::thread(receiveMessages, clientSocket).detach();

    std::string message;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message == "/exit") {
            send(clientSocket, message.c_str(), message.size(), 0);
            break;
        }

        std::string fullMessage = username + ": " + message;
        send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0);
    }

    close(clientSocket);
    return 0;
}

