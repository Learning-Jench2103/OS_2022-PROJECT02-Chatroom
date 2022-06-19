#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "config.hpp"

using namespace std;

typedef struct socket_client_info {
    int descriptor;
    sockaddr_in addr;
} socket_client_info;

typedef vector<socket_client_info *>::iterator socket_client_info_it;

int sockfd = -1;
mutex mu;
vector<socket_client_info *> socket_client_vec;

void quit_handler(int s) {
    for (socket_client_info_it it = socket_client_vec.begin(); it != socket_client_vec.end(); ++it) {
        delete (*it);
    }
    printf("\n");
    exit(0);
}

void client_handler(socket_client_info *client_ptr) {
    char buffer[MAX_MSG_LENGTH];
    int result = 0;

    while (true) {
        bzero(buffer, sizeof(buffer));

        result = recv(client_ptr->descriptor, buffer, sizeof(buffer), 0);
        if (result == -1 || buffer[0] == 0) {
            close(client_ptr->descriptor);
            socket_client_info_it it = find(socket_client_vec.begin(), socket_client_vec.end(), client_ptr);
            delete (*it);
            socket_client_vec.erase(it);
            break;
        }

        mu.lock();
        for (socket_client_info_it it = socket_client_vec.begin(); it != socket_client_vec.end(); ++it) {
            if (*it == client_ptr || (*it)->descriptor == 0) {
                continue;
            }
            send((*it)->descriptor, buffer, sizeof(buffer), 0);
        }
        printf("%s\n", buffer);
        mu.unlock();
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

    string server_ip = argv[1];
    string server_port = argv[2];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET};
    int result = 0;

    if (sockfd == -1) {
        error_handler("Fail to create a socket instance.");
    }

    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    addr.sin_port = htons(stoi(server_port));

    result = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    if (result == -1) {
        error_handler("Fail to bind the ip address.");
    }

    printf("Server started!\n");

    listen(sockfd, MAX_CLIENT);

    while (socket_client_vec.size() <= MAX_CLIENT) {
        socket_client_info *client_ptr = new socket_client_info;
        socket_client_vec.push_back(client_ptr);
        socklen_t addr_len = sizeof(client_ptr->addr);
        client_ptr->descriptor = accept(sockfd, (sockaddr *)&(client_ptr->addr), &addr_len);
        thread client_handler_thread(client_handler, client_ptr);
        client_handler_thread.detach();
    }

    return 0;
}
