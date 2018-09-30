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
#include <sstream>      // to isolate the command typed in

using namespace std;


char packetBuf[4096];
string ID;

string user = "unknown";
bool intialID = true;
string theCommand;
string theRequest;

void error(const char *msg)
{
    perror(msg);
}

string getCommand(string fromCIN) {
    vector<string> splitter;
    string result = fromCIN;

    istringstream iss(result);
    for(string result; iss >> result; )
        splitter.push_back(result);

    theCommand = splitter[0];
}

int port_is_open(struct sockaddr_in address, int sockfd) {

    return connect(sockfd,(struct sockaddr *) &address,sizeof(address));
}

int read_from_client(int socketFD, struct sockaddr_in serv_addr) {
    char buffer[255];
    int numberOfBytes = read(socketFD, buffer, 512);
    theRequest = buffer;

    if(numberOfBytes < 0) {
        error("error reading");
    }
    else if(numberOfBytes == 0) {
        return -1;
    }
    else {
        if(theCommand == "ID"){
            cout<<"Your id is as follows: "<<theRequest<<endl;
            intialID = false;
        }
        else if(theCommand == "CHANGEID") {
            cout<<"Your new id is: " << theRequest<<endl;
        }
        else if(theCommand == "WHO") {
            cout<<"All users: "<< theRequest<<endl;
        }
        else if(theCommand == "CONNECT" && theRequest != "") {
            cout<<"Connect went through"<<endl;
            ID = buffer;
            user = theRequest;
            memset(buffer, 0, sizeof(buffer));
        }
        else if(theCommand == "READ") {
            cout<<"You got a message: " << buffer<<endl;
        }
        else {
            //cout<<"Connect didn't work"<<endl;
        }

        memset(buffer, 0, sizeof(buffer));

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

            theCommand = "";
            cout<<ID<<": ";

            getline(cin, theCommand);
            cout<<endl;

            if(theCommand == "LEAVE") {
                string disco =  "LEAVE " + ID;
                intialID = true;
                char * stringToChar = new char[disco.length()+1];
                strcpy (stringToChar, disco.c_str());
                sendto(sockfd, stringToChar, sizeof(stringToChar), 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
                //ID = "unknown";
                break;
            }
            char * stringToChar = new char[theCommand.length()+1];
            strcpy (stringToChar, theCommand.c_str());
            sendBytes = sendto(sockfd, stringToChar, theCommand.length(), 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
            if(sendBytes < 0) error("send: ");
            getCommand(theCommand);
            read_from_client(sockfd, serv_addr);

        }



    }else{
        cout<<"Port "<<port<<": Closed"<<endl;
    }

    close(sockfd);

    return 0;
}
