#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

void clearConsole() {
    std::cout << "\033[2J\033[1;1H"; // clear the console 
}

void receiveMessages(int clientSocket) {
    char buffer[1024] = {0}; // store incoming messages
    while (true) {
        int valread = read(clientSocket, buffer, 1024); 
        if (valread <= 0) { // error handling
            std::cout << "Disconnected from server" << std::endl;
            close(clientSocket);
            exit(0);
        }
        std::string message(buffer); 
        std::cout << "\n" << message << std::endl; 
        std::cout << "> "; 
        fflush(stdout); 
        memset(buffer, 0, sizeof(buffer)); 
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    const int serverPort = 55554;

    // create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serverAddress.sin_family = AF_INET; // IPv4
    serverAddress.sin_port = htons(serverPort); // set port number

    // convert IPv4 and IPv6 from text to binary 
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(clientSocket);
        return -1;
    }

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        return -1;
    }

    // username
    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    clearConsole(); 
    
    // new thread to receive messages from the server
    std::thread(receiveMessages, clientSocket).detach();

    std::string message;
    while (true) {
        std::cout << "> "; 
        std::getline(std::cin, message); // get message 

        if (message == "/exit") { // exit condition
            send(clientSocket, message.c_str(), message.size(), 0);
            break;
        }

        std::string fullMessage = username + ": " + message; // add user name to the message
        send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0); // send message to server
    }

    close(clientSocket); // close the socket
    return 0;
}

