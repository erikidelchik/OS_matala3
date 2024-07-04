#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;


typedef void* (*reactorFunc)(int fd);

class Reactor {
private:
    fd_set main_set, read_set;
    int fdmax;
    bool stopped;
    unordered_map<int, reactorFunc> funcs;

public:
    int addFd(int fd, reactorFunc func){
        FD_SET(fd, &main_set);
        if (fd > fdmax) {
            fdmax = fd;
        }
        funcs[fd] = func;

        return 0;
    }

    int removeFd(int fd){
        FD_CLR(fd, &main_set);
        funcs.erase(fd);
        return 0;
    }

    int stop(){
        stopped = true;
        return 0;
    }

    void reactorFunction(int serverFd,struct sockaddr_in &address,reactorFunc func) {
        int addrlen = sizeof(address);

        while (!stopped) {
            read_set = main_set;
            select(fdmax + 1, &read_set, nullptr, nullptr, nullptr);

            for (int i = 0; i <= fdmax; ++i) {
                if (FD_ISSET(i, &read_set)) {
                    if(i==serverFd){
                        int newSocket = accept(serverFd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
                        if (newSocket >= 0) {
                            addFd(newSocket,func);
                            cout << "New connection accepted" << endl;
                        }
                    }
                    else {
                        if (funcs.find(i) != funcs.end() && funcs[i]) {
                            funcs[i](i);
                        }
                    }
                }
            }
        }
    }

    Reactor():fdmax(0) {
        stopped = false;
        FD_ZERO(&main_set);
        FD_ZERO(&read_set);
    }

};
