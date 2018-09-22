#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>       // time
#include <stdlib.h>     // srand, rand
#include <thread>       // sleep_for
#include <chrono>       // milliseconds
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector

using namespace std;

void error(const char *msg)
{
    perror(msg);
}

bool port_is_open(struct sockaddr_in address, int sockfd) {

    return connect(sockfd,(struct sockaddr *) &address,sizeof(address)) >= 0;
}

int main(int argc, char *argv[]){

    int startPort = stoi(argv[2]);
    int endPort = stoi(argv[3]);

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open tcp Socket

    server = gethostbyname(argv[1]);

    bzero((char *) &serv_addr, sizeof(serv_addr));//fill the serv_addr with zeros
    serv_addr.sin_family = AF_INET; // This is always set to AF_INET
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);//copy from the server pointers address into the sockaddr_in variable
    int currentPort = 0;

    currentPort = ports.at(i-startPort);
    int port = ports.at(i-startPort);
    serv_addr.sin_port = htons(port);

    if(port_is_open(serv_addr, sockfd)){//call to the boolean function in line 24
        cout<<"Port "<<port<<": Open"<<endl;
    }else{
        cout<<"Port "<<port<<": Closed"<<endl;
    }
    wait_random_time();// call to the function in line 33

    close(sockfd);

    return 0;
}