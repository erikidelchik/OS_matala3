#include <iostream>
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef void* (*proactorFunc)(int fd);

struct ProactorArgs {
    int serverFd;
    proactorFunc threadFunc;
    struct sockaddr_in address;
};

void* proactorFunction(void *arg){
	ProactorArgs* args = static_cast<ProactorArgs*>(arg);

	while (true) {
		socklen_t addrlen = sizeof(args->address);
		int clientFd = accept(args->serverFd, (struct sockaddr*)&(args->address), &addrlen);
		if (clientFd >= 0) {
		    cout << "New connection accepted" << endl;
		    // Start a new thread for each client connection
		    thread(args->threadFunc, clientFd).detach();
		} 
		else {
		    perror("accept");
		}
    	}
    	return nullptr;
    	

}

pthread_t startProactor(int serverFd, proactorFunc threadFunc,struct sockaddr_in &address) {

    pthread_t tid;
    ProactorArgs* args = new ProactorArgs{serverFd, threadFunc, address};
    
    if (pthread_create(&tid, nullptr, proactorFunction, args)) {
        cerr << "Error creating thread" << endl;
        delete args;
        return 0;
    }
    
    
    return tid;
    
}



int stopProactor(pthread_t tid) {
    
     if (pthread_cancel(tid) != 0) {
         perror("pthread_cancel");
         return -1;
     }
     
     // Ensure the thread is cleaned up
     pthread_join(tid, nullptr); 
     return 0;
   
}



