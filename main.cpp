#include <iostream>
#include "kosaraju_vec_edges.hpp"
#include "kosaraju_adj_mat.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "reactor.hpp"
#include <mutex>
#include <thread>
#include "proactor.hpp"
#include <random>


void run_q3();

using namespace std;
GFG_vec_edges gfg;
GFG_adj_mat adj_mat;

pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

mutex mtx;
bool above_half = false;
bool ready = false;
bool happend = false;
bool q10 = false;

void gprof_adj_mat(){
    int n=100;
    int m=100;
    
    random_device rd;
    mt19937 gen(rd());
    
    uniform_int_distribution<> distr(1, 100);
    
    int i,j;
    
    vector<vector<int>> edges;
    
    
    
    for(int k=0;k<100;k++){
    	i = distr(gen);
    	j = distr(gen);
    	edges.push_back({i,j});
    }
    
    adj_mat.findSCC(100,edges);

}

void gprof_vec_edges(){
    
    random_device rd;
    mt19937 gen(rd());
    
    uniform_int_distribution<> distr(1, 100);
    
    int i,j;
    
    gfg.setV(100);
    
    for(int k=0;k<100;k++){
    	i = distr(gen);
    	j = distr(gen);
    	gfg.addEdge({i,j});
    }
    
    gfg.findSCC();
    
    
}

void* notify(void *arg){
    while(true){
    	    pthread_mutex_lock(&mutx);
	    while (!ready) {
		pthread_cond_wait(&cv, &mutx);
	    }
	    
	    cout<<"At Least 50% of the graph belongs to the same SCC\n";
	    
	    ready = false;
	    pthread_mutex_unlock(&mutx);
    }
    return nullptr;
    
}

void setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

void setInput(int n,int m,int fd=-1){

    char buffer[1024];
    
    
    gfg.setV(n);

    for(int k=0;k<m;k++){
    	
    	int i;
        int j;
        
    	if(fd!=-1){
    	    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    	    buffer[bytesRead] = '\0';
    	    string s = buffer;
    	    size_t space = s.find(' ');
    	    i = s.at(space-1) -'0';
    	    j = s.at(space+1) -'0';
    	    
    	
    	}
    	
        else{
            cin>>i>>j;
        
        }
        gfg.addEdge({i,j});
    }
}

void printSCC(int fd=-1){
    vector<vector<int>> ans = gfg.findSCC();
    char buffer[1024];
    string str;
    
    if(fd==-1){
	 for (const vector<int>& x : ans) {
	    for (auto y : x) {
		cout << y << " ";
	    }
	 cout << "\n";
	 }
    }
    else{
    	int vertices = gfg.getV();
    	int lfs = -1;
    	for (const vector<int>& x : ans) {
	    for (auto y : x) {
		str+= to_string(y)+" ";
	    }
	    lfs+=1;
	    str+="\n";
	 }
	 
	 if(vertices/2>=lfs) above_half = true;
	 else above_half = false;
	 
	 if(above_half && !happend && q10){ 
	 	happend = true;
	 	ready = true;
	 	pthread_cond_broadcast(&cv);
	 }
	 
	 else if(!above_half && happend && q10){
	 	happend = false;
	 	cout<<"At Least 50% of the graph no longer belongs to the same SCC\n";
	 	
	 }
	 
	 strcpy(buffer,str.c_str());
	 buffer[str.size()]='\0';
	 write(fd,buffer,strlen(buffer));
    
    }
    
    
    
}

void start_server(int& serverFd,struct sockaddr_in &address){
    int opt = 1;
    int addrlen = sizeof(address);
    // Create socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(1);
    }

    // Set socket options
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(serverFd);
        exit(1);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9034);

    // Bind the socket to the port 9034
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(serverFd);
        exit(1);
    }
    
    

    // Listen for incoming connections
    if (listen(serverFd, 3) < 0) {
        perror("listen");
        close(serverFd);
        exit(1);
    }

    std::cout << "Server listening on port 9034" << std::endl;
}


void* handleClient(int clientFd){
    char buffer[1024];
    string action;
    
    
        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            close(clientFd);
            return nullptr;
        }

        buffer[bytesRead] = '\0';  // Null-terminate the buffer
        action = buffer;
	cout << "Received action: " << action << endl;
	
	
        if(action.find("Newgraph")==0){
            size_t comma = action.find(',');
            cout<<"Newgraph command:\n";
            gfg.clearGraph();
            int n = action.at(comma-1)-'0';
            int m = action.at(comma+1)-'0';
            
            setInput(n,m,clientFd);
            
            cout << "Newgraph with " << n << " vertices and " << m << " edges" << endl;
        }
        else if(action.find("Kosaraju")==0){
            printSCC(clientFd);
        }
        else if(action.find("Newedge")==0){
            cout<<"Newedge command:\n";
            size_t comma = action.find(',');
            vector<int> e;
            int i = action.at(comma-1)-'0';
            int j = action.at(comma+1)-'0';
            e.push_back(i);
            e.push_back(j);
            gfg.addEdge(e);
        }
        else if(action.find("Removeedge")==0){

            cout<<"Removeedge command:\n";
            size_t comma = action.find(',');
            vector<int> e;
            int i = action.at(comma-1)-'0';
            int j = action.at(comma+1)-'0';
            e.push_back(i);
            e.push_back(j);
            gfg.removeEdge(e);
        }
        
        else{
            cout<<"no such command\n";
        }
        
        return nullptr;

}

void* handleClient_thread(int clientFd){
    char buffer[1024];
    string action;
    
    while(true){
        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            close(clientFd);
            return nullptr;
        }

        buffer[bytesRead] = '\0';  // Null-terminate the buffer
        action = buffer;
	cout << "Received action: " << action << endl;
	
	try{
		
		lock_guard<mutex> lock(mtx);
		if(action.find("Newgraph")==0){
		    
		    size_t comma = action.find(',');
		    cout<<"Newgraph command:\n";
		    gfg.clearGraph();
		    int n = action.at(comma-1)-'0';
		    int m = action.at(comma+1)-'0';
		    
		    setInput(n,m,clientFd);
		    
		    cout << "Newgraph with " << n << " vertices and " << m << " edges" << endl;
		}
		else if(action.find("Kosaraju")==0){
		    
		    printSCC(clientFd);
		}
		else if(action.find("Newedge")==0){
		    
		    cout<<"Newedge command:\n";
		    size_t comma = action.find(',');
		    vector<int> e;
		    int i = action.at(comma-1)-'0';
		    int j = action.at(comma+1)-'0';
		    e.push_back(i);
		    e.push_back(j);
		    gfg.addEdge(e);
		}
		else if(action.find("Removeedge")==0){
		   
		    
		    cout<<"Removeedge command:\n";
		    size_t comma = action.find(',');
		    vector<int> e;
		    int i = action.at(comma-1)-'0';
		    int j = action.at(comma+1)-'0';
		    e.push_back(i);
		    e.push_back(j);
		    gfg.removeEdge(e);
		}
		
		else{
		    cout<<"no such command\n";
		}
	}
	catch(const std::exception& e){
	    cout<<e.what()<<endl;
	}
        
    }
        
    return nullptr;
    
    
    
}


void run_q1(){
    int n;
    int m;

    cin>>n>>m;
    setInput(n,m);
    printSCC();
}

void run_q3(){
    string action;

    while(true){
        getline(cin,action);

        if(action.find("Newgraph")==0){
            gfg.clearGraph();
            int n = action.at(action.size()-3)-'0';
            int m = action.at(action.size()-1)-'0';
            setInput(n,m);
            cout << "Newgraph with " << n << " vertices and " << m << " edges" << endl;
        }
        else if(action.find("Kosaraju")==0){
            printSCC();
        }
        else if(action.find("Newedge")==0){
            vector<int> e;
            int i = action.at(action.size()-3)-'0';
            int j = action.at(action.size()-1)-'0';
            e.push_back(i);
            e.push_back(j);
            gfg.addEdge(e);
        }
        else if(action.find("Removeedge")==0){
            vector<int> e;
            int i = action.at(action.size()-3)-'0';
            int j = action.at(action.size()-1)-'0';
            e.push_back(i);
            e.push_back(j);
            gfg.removeEdge(e);
        }
        

    }


}

void run_q4(){
    int serverFd = -1;
    int fdmax;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    fd_set read_set, main_set;

    start_server(serverFd,address);
    setNonBlocking(serverFd);
    
    // Initialize the set of fds
    FD_ZERO(&read_set);
    FD_ZERO(&main_set);
    
    // Add the server fd to the set
    FD_SET(serverFd, &main_set);
    fdmax = serverFd;

    while(true) {
        read_set = main_set;
        select(fdmax + 1, &read_set, nullptr, nullptr, nullptr);
    
    	
    	for(int i=0;i<=fdmax;i++){
    	    if (FD_ISSET(i, &read_set)){
    	        if(i==serverFd){
    	            int newSocket = accept(serverFd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
        	    if (newSocket >= 0) {
        	    	FD_SET(newSocket,&main_set);
        	    	if(newSocket>fdmax){
        	    	    fdmax = newSocket;
        	    	}
            		cout << "New connection accepted" << endl;   
        	    }
    	        
    	        }
    	        
    	        else{
    	            handleClient(i);
    	    
    	    	}	
    	    }
    	}

    }
    close(serverFd);

}


void run_q6(){
    int serverFd = -1;
    struct sockaddr_in address;
    start_server(serverFd,address);
    setNonBlocking(serverFd);
    
    Reactor reactor;
    reactorFunc f = handleClient;
    
    reactor.addFd(serverFd,nullptr);
    
    
    
    reactor.reactorFunction(serverFd,address,f);
    
    reactor.stop();
    
    close(serverFd);

}


void run_q7(){
    int serverFd = -1;
    struct sockaddr_in address;

    start_server(serverFd, address);

    while (true) {
        socklen_t addrlen = sizeof(address);
        int clientFd = accept(serverFd, (struct sockaddr*)&address, &addrlen);
        if (clientFd >= 0) {
            cout << "New connection accepted" << endl;
            // Start a new thread for each client connection
            thread(handleClient_thread, clientFd).detach();
        } else {
            perror("accept");
        }
    }

    close(serverFd);

}

void run_q9(){
    q10 = true;
    int serverFd = -1;
    struct sockaddr_in address;
    
    start_server(serverFd, address);
    
    proactorFunc f = handleClient_thread;
    
    pthread_t pid = startProactor(serverFd,f,address);
    
    pthread_t notifier_thread;
    pthread_create(&notifier_thread, nullptr, notify, nullptr);
    
    pthread_join(pid, nullptr);

}


int main() {
    gprof_adj_mat();

    return 0;
}
