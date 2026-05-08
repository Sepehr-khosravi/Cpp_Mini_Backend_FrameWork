#include <iostream>
#include <string>
#include <sys/wait.h>
#include <cstdlib>
using namespace std;
#include "address.hpp"
#include <signal.h>

int main() {
    std::string server_path = getServerAddress();
    
    std::cout << "Cleaning up old server processes..." << std::endl;
    system("pkill -f \"build/run\" 2>/dev/null && clear");
    sleep(1);  
    
    while (true) {
        pid_t server_pid = fork();
        
        if (server_pid == 0) {
            execl("/bin/sh", "sh", "-c", server_path.c_str(), nullptr);
            exit(1);
        } else if (server_pid > 0) {
            int status;
            waitpid(server_pid, &status, 0);
            
            std::cout << "Server died with status: " << status << std::endl;
            std::cout << "Restarting in 2 seconds..." << std::endl;
            sleep(2);
            system("clear");
        }
    }
    
    return 0;
}
