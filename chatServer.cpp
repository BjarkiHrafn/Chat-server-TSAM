/* helpful links
https://linux.die.net/man/2/socket

client and server example:
http://www.linuxhowtos.org/C_C++/socket.htm

http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <array>
#include <string.h> // for bzero
#include <fstream> // fpr read

#include <arpa/inet.h> //inet_ntoa

#include <stdlib.h>
#include <unistd.h> // close()


#include <ctime> // for the timestamp
#include <sstream>
#include <iomanip>
#include <vector> //fore the username vector
#include <algorithm> // for the username vector

using namespace std;

string msgBuf;
vector<string> users;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

string provide_unique_id() {
    //group = Y_Project_2 42 Y_Project_2
    int sockFd;
    struct sockaddr_in serverAddress;
    string command = "fortune -s";
    string result;
    array<char, 128> buffer;

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
       std::cerr << "Couldn't start command." << std::endl;
       return 0;
    }
    while (fgets(buffer.data(), 128, pipe) != NULL) {
       std::cout << "Reading..." << std::endl;
       result += buffer.data();
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto str = oss.str();

    result = result + " Y_Project_2 42" + str;

    return result;
}

int createSocket(int socketFileDescriptor) {
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0) {
         error("ERROR opening socket");
    }
    return socketFileDescriptor;
}

int bindSocket(int socketFileDescriptor, struct sockaddr_in serverAddress) {
    int bindSuccess = bind(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(bindSuccess < 0) {
      error("Error binding socket");
    }
    return bindSuccess;
}

int selectFileDescritporSet(fd_set FDSet) {
    /*
    allow a program to monitor multiple file
    descriptors, waiting until one or more of the file descriptors become
    "ready" for some class of I/O operation (e.g., input possible).
    */
    cout<< "Starting Select routine"<<endl;
    int numberOfFileDescriptors = select(FD_SETSIZE, &FDSet, NULL, NULL, NULL);
    if (numberOfFileDescriptors < 0) {
        error("Error selecting");
    }
    cout<< "Recieved a message."<<endl;
    return numberOfFileDescriptors;
}

int read_from_client(int socketFD) {
    char buffer[255];

    int numberOfBytes = read(socketFD, buffer, 512);

    if(numberOfBytes < 0) {
        error("error reading");
    }
    else if(numberOfBytes == 0) {
        return -1;
    }
    else {
        msgBuf += buffer;
        msgBuf += " ";
        cout<<"msgBuf is: "<< msgBuf<<endl;
        //strcmp(msgBuf, buffer);
        cout << stderr << "Server: got message: " << buffer << endl;
        return 0;
    }
}

void arePortsOpen(int &socketFD, int startPort, int endPort) {
    struct sockaddr_in serverAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    for(int i = startPort; i <= endPort; i++) {
        serverAddress.sin_port = htons(i);
        if(connect(socketFD,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) >= 0) {
            cout<<"Port: "<< i <<" is open"<<endl;
        }
        else {
            error("Port closed");
        }
    }
}

void checkCommandAndReact(int &newSocketFD1, struct sockaddr_in& clientAddress) {
    char buffer[5000];

    string result = msgBuf;
    vector<string> splitter;
    istringstream iss(result);
    for(string result; iss >> result; )
        splitter.push_back(result);

    if(splitter[0] == "ID")
        result = provide_unique_id();
    else if(splitter[0] == "CONNECT" && splitter.size() > 1) {
        if(find(users.begin(), users.end(), splitter[1]) == users.end()) {
            users.push_back(splitter[1]);
            result = splitter[1];
            msgBuf = "";

        } else {
            result = "F";
        }
    }
    else if(splitter[0] == "LEAVE") {
        cout<<splitter[1]<<" wants to leave!!"<<endl;
    }
    char * stringToChar = new char[result.length()+1];
    //cout<<"splitter[0]: "<< splitter[0]<< " splitter[1]: "<< splitter[1]<<endl;
    strcpy (stringToChar, result.c_str());
    sendto(newSocketFD1, stringToChar, 200, 0, (struct sockaddr*)&clientAddress, sizeof clientAddress);
}

int main(int argc, char *argv[]) {


    //if(strcmp(argv[1], "ID") == 0)
    //    cout << provide_unique_id() << endl;
    //select 3 consecutive ports for port knocking
    int portNumber1 = 50040;
    int portNumber2 = 50041;
    int portNumber3 = 50042;

    // initalize File Descriptors
    int socketFD1 = 0; // main socket
    int socketFD2 = 0;
    int socketFD3 = 0;

    int bindSuccess = 0;

    int newSocketFD1 = 0;

    // create set of file descritor sets
    fd_set activeFDSet;
    fd_set readFDSet;

    int returnValue = 0; // return value for the read() and write() calls; i.e. it contains the number of characters read or written.
    socklen_t clilen = 0; // stores the size of the address of the client. This is needed for the accept system call.

    struct sockaddr_in serverAddress1;
    struct sockaddr_in serverAddress2;
    struct sockaddr_in serverAddress3;
    struct sockaddr_in clientAddress;

    socketFD1 = createSocket(socketFD1);
    socketFD2 = createSocket(socketFD2);
    socketFD3 = createSocket(socketFD3);

    bzero((char *) &serverAddress1, sizeof(serverAddress1)); // initializes serverAddress to zeros.
    bzero((char *) &serverAddress2, sizeof(serverAddress2));
    bzero((char *) &serverAddress3, sizeof(serverAddress3));

    serverAddress1.sin_family = AF_INET; // code for address family, this is always set to AF_INET
    serverAddress2.sin_family = AF_INET;
    serverAddress3.sin_family = AF_INET;

    serverAddress1.sin_addr.s_addr = INADDR_ANY; // this will always be the IP address of the machine on which the server is running
    serverAddress2.sin_addr.s_addr = INADDR_ANY;
    serverAddress3.sin_addr.s_addr = INADDR_ANY;

    // check if ports are open
    //arePortsOpen(socketFD1, portNumber1, portNumber3);

    serverAddress1.sin_port = htons(portNumber1);
    serverAddress2.sin_port = htons(portNumber2);
    serverAddress3.sin_port = htons(portNumber3);
    //serverAddress.sin_port = htons(8001);
    bindSocket(socketFD1, serverAddress1);
    bindSocket(socketFD2, serverAddress2);
    bindSocket(socketFD3, serverAddress3);

    // Initialize the set of active sockets.
    //socketFD1 = createSocket(socketFD1);
    FD_ZERO (&activeFDSet);
    FD_SET (socketFD1, &activeFDSet);

    //cout<< stoi(activeFDSet)<<endl;
    if(listen(socketFD3, 5) >= 0) {
        cout<<"socketFD3 went through"<<endl;
        if(listen(socketFD1, 5) >= 0) {
            cout<<"socketFD2 went through" <<endl;
            if(listen(socketFD2, 5) >= 0) {
                cout<<"Listening..."<<endl;
            } else {error("Listen");}
        } else {error("socket1");}
    } else {error("socket3");}

    /*if (listen (socketFD1, 5) < 0) {
    error ("listen");
    exit (EXIT_FAILURE);
    } else {
        cout<< "Listening..."<<endl;
    }*/

    while(1) {
        // Block untill input arrives on one or more of the active sockets
        readFDSet = activeFDSet;
        //selectFileDescritporSet(readFDSet);

        int selectValue = select(FD_SETSIZE, &readFDSet, NULL, NULL, NULL);
        if(selectValue < 0) error("Select");
        cout<<"Done selecting :)"<<endl;
        for(int i = 0; i < FD_SETSIZE; i++) {
            // check if file descriptor is part of the set
            if(FD_ISSET(i, &readFDSet)) {
                if(i == socketFD1) {
                    clilen = sizeof(clientAddress);
                    /*extract the first connection on the queue of pending connections,
                    create a new socket with the same socket type protocol
                    and address family as the specified socket,
                    and allocate a new file descriptor for that socket.*/

                    newSocketFD1 = accept(socketFD1, (struct sockaddr *) &clientAddress, &clilen);
                    if(newSocketFD1 < 0) {
                        error("accept error");
                    }

                    fprintf (stderr,
                             "Server: connect from host %s, port %hd.\n",
                             inet_ntoa (clientAddress.sin_addr),
                             ntohs (clientAddress.sin_port));
                    FD_SET (newSocketFD1, &activeFDSet);

                }
                else {
                    // data arriving on a connected socket

                    if (read_from_client (i) < 0) {
                        close (i);
                        FD_CLR (i, &activeFDSet);
                    }

                    checkCommandAndReact(newSocketFD1, clientAddress);

                }
            }
        }
    }
    return 0;
}
