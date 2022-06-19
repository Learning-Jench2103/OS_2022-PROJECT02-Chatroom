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

int sockfd = -1;
mutex mu;
vector<SocketClientInfo *> socket_client_vec;

void quit_handler(int s) {
    for (SocketClientInfoIterator it = socket_client_vec.begin(); it != socket_client_vec.end(); ++it) {
        delete (*it);
    }
    printf("\n");
    exit(0);
}

void client_handler(SocketClientInfo *client_ptr) {
    char buffer[MAX_MSG_LENGTH];
    int result = 0;

    while (true) {
        bzero(buffer, sizeof(buffer));

        result = recv(client_ptr->descriptor, buffer, sizeof(buffer), 0);
        if (result == -1 || buffer[0] == 0) {
            close(client_ptr->descriptor);
            SocketClientInfoIterator it = find(socket_client_vec.begin(), socket_client_vec.end(), client_ptr);
            delete (*it);
            socket_client_vec.erase(it);
            break;
        }

        mu.lock();
        for (SocketClientInfoIterator it = socket_client_vec.begin(); it != socket_client_vec.end(); ++it) {
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
    WebAddr web_addr;
    if (argc != 2 || !check_web_addr(string(argv[1]), &web_addr)) {
        error_handler("Please pass the address of the server in the format of \"<IP ADDRESS>:<PORT>\".", 0);
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = quit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET};
    int result = 0;

    if (sockfd == -1) {
        error_handler("Fail to create a socket instance.", 0);
    }

    addr.sin_addr.s_addr = web_addr.addr;
    addr.sin_port = web_addr.port;

    result = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    if (result == -1) {
        error_handler("Fail to bind the ip address.", 0);
    }

    printf("Server started!\n");

    listen(sockfd, MAX_CLIENT);

    while (socket_client_vec.size() <= MAX_CLIENT) {
        SocketClientInfo *client_ptr = new SocketClientInfo;
        socket_client_vec.push_back(client_ptr);
        socklen_t addr_len = sizeof(client_ptr->addr);
        client_ptr->descriptor = accept(sockfd, (sockaddr *)&(client_ptr->addr), &addr_len);
        thread client_handler_thread(client_handler, client_ptr);
        client_handler_thread.detach();
    }

    return 0;
}
