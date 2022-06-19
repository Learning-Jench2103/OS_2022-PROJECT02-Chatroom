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
        // receive messages from the server
        result = recv(sockfd, receive_buffer, sizeof(receive_buffer), 0);
        if (result == -1 || result == 0 || receive_buffer[0] == 0) {
            // terminate the process when the server disconnected
            error_handler("Fail to connect with the server.", 1);
        }
        printf("%s\n", receive_buffer);
    }

    return;
}

int main(int argc, char **argv) {
    // check the format of the argument and parse it
    WebAddr web_addr;
    if (argc != 2 || !check_web_addr(string(argv[1]), &web_addr)) {
        error_handler("Please pass the address of the server in the format of \"<IP ADDRESS>:<PORT>\".", 0);
    }

    // handle ctrl + c termination
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = quit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // create socket and message buffer
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET};
    char send_buffer[MAX_MSG_LENGTH];
    int result = 0;

    // connect the socket instance to the server
    addr.sin_addr.s_addr = web_addr.addr;
    addr.sin_port = web_addr.port;

    result = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1) {
        error_handler("Fail to connect to the server.", 0);
    }

    // set the username
    printf("Please enter your username: ");
    getline(cin, username);

    sprintf(send_buffer, "%s joined the chat room.", username.c_str());
    result = send(sockfd, send_buffer, strlen(send_buffer), 0);
    if (result == -1) {
        error_handler("Fail to send the message.", 1);
    }

    // welcome message
    printf("\nWelcome %s!\n\n", username.c_str());
    printf("You can write a message and press enter to send it to everyone,\n");
    printf("or key in \'quit\' to leave the chat room.\n\n");

    // create a thread to handle messages from the server
    thread receiver_thread(receiver, sockfd);
    receiver_thread.detach();

    while (true) {
        string input;
        getline(cin, input);

        // handle "quit" command
        if (input == string("quit")) {
            printf("\nGoodbye, %s!\n", username.c_str());
            return 0;
        }

        // prevent the program from sending empty messages
        if (input.length() == 0) {
            continue;
        }

        // send the new message to server
        sprintf(send_buffer, "%s: %s", username.c_str(), input.c_str());
        result = send(sockfd, send_buffer, strlen(send_buffer), 0);
        if (result == -1) {
            error_handler("Fail to send the message.", 1);
        }
    }

    return 0;
}
