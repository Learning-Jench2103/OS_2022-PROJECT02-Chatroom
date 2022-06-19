#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "config.hpp"

using namespace std;

string username = "anonymous";

void quit_handler(int s) {
    printf("\nGoodbye, %s!\n", username.c_str());
    exit(0);
}

void receiver(int sockfd) {
    char receive_buffer[MAX_MSG_LENGTH] = {};
    int result = 0;

    while (true) {
        result = recv(sockfd, receive_buffer, sizeof(receive_buffer), 0);
        if (result == -1 || receive_buffer[0] == 0) {
            error_handler("Fail to connect with the server.");
        }
        printf("%s\n", receive_buffer);
    }

    return;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        error_handler("Please pass just 2 parameters as IP ADDRESS and PORT.");
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = quit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET};
    char send_buffer[MAX_MSG_LENGTH];
    string server_ip = argv[1];
    string server_port = argv[2];
    int result = 0;

    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    addr.sin_port = htons(stoi(server_port));

    result = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1) {
        error_handler("Fail to connect with the server.");
    }

    printf("Please enter your username: ");
    getline(cin, username);

    sprintf(send_buffer, "%s joined the chat room.", username.c_str());
    result = send(sockfd, send_buffer, strlen(send_buffer), 0);
    if (result == -1) {
        error_handler("Fail to send the message.");
    }

    printf("\nWelcome %s!\n\n", username.c_str());
    printf("You can write a message and press enter to send it to everyone,\n");
    printf("or key in \'quit\' to leave the chat room.\n\n");

    thread receiver_thread(receiver, sockfd);
    receiver_thread.detach();

    while (true) {
        string input;
        getline(cin, input);
        if (input == string("quit")) {
            printf("\nGoodbye, %s!\n", username.c_str());
            return 0;
        }

        sprintf(send_buffer, "%s: %s", username.c_str(), input.c_str());
        result = send(sockfd, send_buffer, strlen(send_buffer), 0);
        if (result == -1) {
            error_handler("Fail to send the message.");
        }
    }

    return 0;
}
