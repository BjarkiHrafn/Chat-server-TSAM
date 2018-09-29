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
#include <map>

 #include <sys/ioctl.h> // ioctl

using namespace std;

struct Knock {
    vector<int> ports;
    string connectTime;
} ;

//select 3 consecutive ports for port knocking
int portNumber1 = 50040;
int portNumber2 = 50041;
int portNumber3 = 50042;

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
  // SOCK_NONBLOCK
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFileDescriptor < 0) {
         error("ERROR opening socket");
    }
    return socketFileDescriptor;
}

int bindSocket(int socketFileDescriptor, struct sockaddr_in &serverAddress) {
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

void arePortsOpen(struct sockaddr_in serverAddress, int socketFD, int startPort, int endPort) {

    /*for(int i = startPort; i <= endPort; i++) {
        serverAddress.sin_port = i;
        if(connect(socketFD,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0) {
            cout<<"Port: "<< i <<" is closed"<<endl;
        }
        else {
            cout << i << endl;
            error("Port is open");
        }
    }*/
}

string getCurrTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto str = oss.str();
    return str;
}

bool correctKnock(Knock x) {

    if(x.ports.size() != 3)
    {
      cout << "KNOCK INCOMPLETE" << endl;
      return false;
    }

    string currentTime = getCurrTimestamp();

    //compare everything exept minutes and seconds
    for(int i = 0; i < currentTime.size()-5; i++) {
        if(currentTime[i] != x.connectTime[i]) {
            return false; // we treat the port knock as incorrect if it doesnt happen within a minute
        }
    }

    vector<int> temp = x.ports;
    // hardcoded port knock
    return (temp[0] == portNumber2 && temp[1] == portNumber3 && temp[2] == portNumber1);
}

void updateKnockMap(struct sockaddr_in clientAddress, map<string, Knock> &knockMap, int port) {
    // if the portvector contains three ports then erase the data
    if(knockMap.count(inet_ntoa(clientAddress.sin_addr)) > 0) {
        if(knockMap[inet_ntoa(clientAddress.sin_addr)].ports.size() >= 3) {
            knockMap.erase(inet_ntoa(clientAddress.sin_addr));
        }
    }

    if(knockMap.count(inet_ntoa(clientAddress.sin_addr)) > 0) {
        cout << endl << "I EXIST: " << knockMap.count(inet_ntoa(clientAddress.sin_addr)) << endl << endl;
        knockMap[inet_ntoa(clientAddress.sin_addr)].ports.push_back(port);
    }
    else {
        cout << endl << "I DONT EXIST" << endl << endl;
        pair<string, Knock> x;
        x.first = inet_ntoa(clientAddress.sin_addr);
        x.second.ports.push_back(port);
        x.second.connectTime = getCurrTimestamp();
        cout << "POPULATED CONNECT TIME: " << x.second.connectTime << " !!!" << endl;
        knockMap.insert(x);
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

    map<string, Knock> knockMap;
    //if(strcmp(argv[1], "ID") == 0)
    //    cout << provide_unique_id() << endl;

    // initalize File Descriptors
    int socketFD1 = 0; // main socket
    int socketFD2 = 0;
    int socketFD3 = 0;

    int bindSuccess = 0;
    int portConnectedTo = 0; // the port thie client is connected to

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

    // make socket nonblocking
    int q = 1;
    ioctl(socketFD1, FIONBIO, (char *)&q);
    ioctl(socketFD2, FIONBIO, (char *)&q);
    ioctl(socketFD3, FIONBIO, (char *)&q);

    // make sockets reusable
    setsockopt(socketFD1, SOL_SOCKET, SO_REUSEADDR, &q, sizeof(q));
    setsockopt(socketFD2, SOL_SOCKET, SO_REUSEADDR, &q, sizeof(q));
    setsockopt(socketFD3, SOL_SOCKET, SO_REUSEADDR, &q, sizeof(q));

    bzero((char *) &serverAddress1, sizeof(serverAddress1)); // initializes serverAddress to zeros.
    bzero((char *) &serverAddress2, sizeof(serverAddress2));
    bzero((char *) &serverAddress3, sizeof(serverAddress3));

    serverAddress1.sin_family = AF_INET; // code for address family, this is always set to AF_INET
    serverAddress2.sin_family = AF_INET;
    serverAddress3.sin_family = AF_INET;

    serverAddress1.sin_addr.s_addr = INADDR_ANY; // this will always be the IP address of the machine on which the server is running
    serverAddress2.sin_addr.s_addr = INADDR_ANY;
    serverAddress3.sin_addr.s_addr = INADDR_ANY;

    // when the client is startet we need to pass in a range of ports
    // that we want to ceck are open. if we find 3 consecutive port on
    // that range then those are the ports we will use
    arePortsOpen(serverAddress1, socketFD1, portNumber1, portNumber3);

    serverAddress1.sin_port = htons(portNumber1);
    serverAddress2.sin_port = htons(portNumber2);
    serverAddress3.sin_port = htons(portNumber3);

    cout << "ports set" << endl;
    //serverAddress.sin_port = htons(8001);
    bindSocket(socketFD1, serverAddress1);
    bindSocket(socketFD2, serverAddress2);
    bindSocket(socketFD3, serverAddress3);

    /*bind(socketFD1, (struct sockaddr *) &serverAddress1, sizeof(serverAddress1));
    bind(socketFD2, (struct sockaddr *) &serverAddress2, sizeof(serverAddress2));
    bind(socketFD3, (struct sockaddr *) &serverAddress3, sizeof(serverAddress3));*/

    cout << "sets set" << endl;
    if(listen(socketFD1, 5) < 0) {
        cout << "error is below" << endl;
        error("socket1");
        cout << "nvm" << endl;
    }
    if(listen(socketFD2, 5) < 0) {
        error("socket2");
    }
    if(listen(socketFD3, 5) < 0) {
        error("socket3");
    }
    cout << "hello?" << endl;

    // Initialize the set of active sockets.
    //socketFD1 = createSocket(socketFD1);
    FD_ZERO (&activeFDSet);
    FD_SET (socketFD1, &activeFDSet);
    FD_SET (socketFD2, &activeFDSet);
    FD_SET (socketFD3, &activeFDSet);

    while(1) {
        cout << "inside while loop" << endl;
        // Block untill input arrives on one or more of the active sockets
        readFDSet = activeFDSet;
        //selectFileDescritporSet(readFDSet);
        int selectValue = select(FD_SETSIZE, &readFDSet, NULL, NULL, NULL);
        if(selectValue < 0) error("Select");
        cout<<"Done selecting :)"<<endl;
        for(int i = 0; i < FD_SETSIZE; i++) {
            // check if file descriptor is part of the set
            if(FD_ISSET(i, &readFDSet)) {
                if(i == socketFD1 || i == socketFD2 || i == socketFD3) {
                    clilen = sizeof(clientAddress);
                    /*extract the first connection on the queue of pending connections,
                    create a new socket with the same socket type protocol
                    and address family as the specified socket,
                    and allocate a new file descriptor for that socket.*/

                    newSocketFD1 = accept(i, (struct sockaddr *) &clientAddress, &clilen);
                    if(newSocketFD1 < 0) {
                        error("accept error");
                    }

                    cout << "second output" <<endl;
                    fprintf (stderr,
                             "Server: connect from host %s, port %hd.\n",
                             inet_ntoa (clientAddress.sin_addr),
                             ntohs (clientAddress.sin_port));

                   if(i == socketFD1)
                       portConnectedTo = portNumber1;
                   else if(i == socketFD2)
                       portConnectedTo = portNumber2;
                   else
                       portConnectedTo = portNumber3;

                   updateKnockMap(clientAddress, knockMap, portConnectedTo);
                   Knock temp = knockMap[inet_ntoa(clientAddress.sin_addr)];

                   cout << "time for check" << endl;
                   if(correctKnock(temp)){
                       cout << "---CORRECT KNOCK---" << endl;
                       FD_SET (newSocketFD1, &activeFDSet);
                   }
                   else {
                       cout << "---INCORRECT KNOCK---" << endl;
                       close (newSocketFD1);
                       //FD_CLR (i, &activeFDSet);
                       cout << "done closeing1" << endl;
                   }
                }
                else {

                    cout << "read message" << endl;
                    if (read_from_client (i) < 0) {
                        close (i);
                        FD_CLR (i, &activeFDSet);
                    }

                    checkCommandAndReact(i, clientAddress);
                }
            }
        }
    }
    return 0;
}
