#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

#define MAX_CLIENT 30
#define MAX_MSG_LENGTH 256

using namespace std;

void error_handler(string msg) {
    cout << "\n\nError: " << msg << endl << endl;
    exit(1);
}

#endif
