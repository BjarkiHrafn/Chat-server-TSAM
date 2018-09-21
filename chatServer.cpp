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

using namespace std;

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
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if (socketFileDescriptor < 0) {
         error("ERROR opening socket");
    }
    return socketFileDescriptor;
}

int bindSocket(int socketFileDescriptor, struct sockaddr_in serverAddress) {
    int bindFD1 = bind(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(bindFD1 < 0) {
      error("Error binding socket");
    }
    return bindFD1;
}

int selectFileDescritporSet(fd_set FDSet) {
    int selectDescriptor = select(FD_SETSIZE, &FDSet, NULL, NULL, NULL);
    if (selectDescriptor < 0) {
        error("Error selecting");
    }
}

int main(int argc, char *argv[]) {

    cout << provide_unique_id();

    //select 3 consecutive ports for port knocking
    int portNumber1 = 50040;
    int portNumber2 = 50041;
    int portNumber3 = 50042;

    // initalize File Descriptors
    int socketFD1 = 0;
    //int socketFileDescriptor2 = 0;
    //int socketFileDescriptor3 = 0;*/

    int bindFD1 = 0;

    int newSocketFD1 = 0;

    // create set of file descritor sets
    fd_set activeFDSet;
    fd_set readFDSet;

    int returnValue = 0; // return value for the read() and write() calls; i.e. it contains the number of characters read or written.
    socklen_t clilen = 0; // stores the size of the address of the client. This is needed for the accept system call.

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    createSocket(socketFD1);

    bzero((char *) &serverAddress, sizeof(serverAddress)); // initializes serverAddress to zeros.
    serverAddress.sin_family = AF_INET; // code for address family, this is always set to AF_INET
    serverAddress.sin_addr.s_addr = INADDR_ANY; // this will always be the IP address of the machine on which the server is running
    serverAddress.sin_port = portNumber1;

    bindFD1 = bindSocket(socketFD1, serverAddress);

    // Initialize the set of active sockets.
    FD_ZERO (&activeFDSet);
    FD_SET (socketFD1, &activeFDSet);

    while(1) {
        // Block untill input arrives on one or more of the active sockets
        readFDSet = activeFDSet;
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
                        error("accept errror");
                    }

                    cout << stderr << "Server: connect from host %s, port %hd." << endl;
                    cout << inet_ntoa (clientAddress.sin_addr) << clientAddress.sin_port << endl;
                }
                else {
                    // data arriving on a connected socket
                    //if (read_from_client (i) < 0) {
                        close (i);
                        FD_CLR (i, &activeFDSet);
                    //}
                }
            }
        }
    }

    return 0;
}