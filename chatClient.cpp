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


char packetBuf[4096];
string ID;
string oldMsg = "";

void error(const char *msg)
{
    perror(msg);
}

int port_is_open(struct sockaddr_in address, int sockfd) {

    return connect(sockfd,(struct sockaddr *) &address,sizeof(address));
}

int read_from_client(int socketFD) {
    char buffer[255];
    int numberOfBytes = read(socketFD, buffer, 512);
    char *bfs = buffer;
    string connect = "CONNECT";
    bool trew = (connect == oldMsg);
    oldMsg.erase(oldMsg.find_last_not_of(" ") + 1);

    cout<<"trew: "<< trew<<endl;

    cout<<"connect: "<< connect<<endl;
    cout<<"Old:."<< oldMsg<<"."<<endl;
    cout<<"New: "<< buffer<<endl;

    if(numberOfBytes < 0) {
        error("error reading");
    }
    else if(numberOfBytes == 0) {
        return -1;
    }
    else {
        if(strcasecmp("ID", packetBuf) == 0){
            cout<<buffer<<endl;
        }
        else if(connect == oldMsg) {
            cout<<"Connect went through"<<endl;
            ID = buffer;
            memset(buffer, 0, sizeof(buffer));

        } else {
            cout<<"Connect didn't work"<<endl;
        }

        cout << stderr << "client: got message: `%s'" << buffer << endl;
        oldMsg = buffer;
        memset(buffer, 0, sizeof(buffer));
        //strcpy(buffer, oldMsg);
        return 0;
    }

}

int main(int argc, char *argv[]){

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int port = stoi(argv[2]);

    int sendBytes;
    ID = "unknown";



    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open tcp Socket

    server = gethostbyname(argv[1]);

    bzero((char *) &serv_addr, sizeof(serv_addr));//fill the serv_addr with zeros
    serv_addr.sin_family = AF_INET; // This is always set to AF_INET
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);//copy from the server pointers address into the sockaddr_in variable

    serv_addr.sin_port = htons(port);

    if(port_is_open(serv_addr, sockfd) == 0){
        cout<<"Port "<<port<<": Open"<<endl;
        while(1) {
            cout<<ID<<": ";
            cin >> packetBuf;
            cout<<endl;
            if(strcasecmp("LEAVE", packetBuf) == 0) {
                string disco = disco + "LEAVE" + " " + ID;
                char * stringToChar = new char[disco.length()+1];
                strcpy (stringToChar, disco.c_str());
                sendto(sockfd, stringToChar, 200, 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
                break;
            }

            sendBytes = sendto(sockfd, packetBuf, 200, 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
            if(sendBytes < 0) error("send: ");
            read_from_client(sockfd);
        }



    }else{
        cout<<"Port "<<port<<": Closed"<<endl;
    }

    close(sockfd);

    return 0;
}
