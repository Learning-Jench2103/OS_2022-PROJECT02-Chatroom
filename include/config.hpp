#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <regex>
#include <string>

#define MAX_CLIENT 30
#define MAX_MSG_LENGTH 256

using namespace std;

typedef struct WebAddr {
    in_addr_t addr;
    in_port_t port;
} WebAddr;

typedef struct SocketClientInfo {
    int descriptor;
    sockaddr_in addr;
} SocketClientInfo;

typedef vector<SocketClientInfo*>::iterator SocketClientInfoIterator;

void error_handler(string msg, size_t endline) {
    cout << string(endline, '\n') << "Error: " << msg << endl << endl;
    exit(1);
}

bool check_web_addr(string input, WebAddr* web_addr) {
    regex reg("^(\\d+\\.\\d+\\.\\d+\\.\\d+):(\\d+)$");
    sregex_iterator result(input.begin(), input.end(), reg);
    if (result == sregex_iterator()) {
        return false;
    }

    smatch match = *result;
    web_addr->addr = inet_addr(match.str(1).c_str());
    web_addr->port = htons(stoi(match.str(2)));
    return true;
}

#endif
