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

struct clientInfo {
    int sockfd;
    struct sockaddr_in clientAddress;
} ;

struct Knock {
    vector<int> ports;
    string connectTime;
} ;

//select 3 consecutive ports for port knocking
int portNumber1 = 50040;
int portNumber2 = 50041;
int portNumber3 = 50042;

bool isMessage;
bool isCommand = true;
bool isRequest = true;
bool first = true;

string msgUser;
string msgBuf;
string serverID;
//string theCommand;
string theRequest;
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

    result = result + " Y_Project_2 42 " + str;

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
    char buffer[2000];
    int numberOfBytes = read(socketFD, buffer, 2000);
    if(numberOfBytes < 0) {
        error("error reading");
    }
    else if(numberOfBytes == 0) {
        return -1;
    }
    else {
        msgBuf = buffer;
        memset(buffer, 0, sizeof buffer);
        return 0;
    }
}

void findAvailablePorts(int startPort, int endPort) {
    //initalize test variables
    struct sockaddr_in serverAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress)); // initializes serverAddress to zeros.
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    int testSocketFD = createSocket(testSocketFD);

    // counters used for checking groups of three
    int counter = 0;
    for(int i = startPort; i <= endPort; i++) {
        counter++;
        // set the port which wil be checked
        serverAddress.sin_port = i;
        // check if port is being used
        if(connect(testSocketFD,(struct sockaddr *) &serverAddress,sizeof(serverAddress) < 0)) {
            // check if we have found the consecutive unused ports
            if(counter%3 == 0) {
                portNumber3 = i;
                portNumber2 = i - 1;
                portNumber1 = i - 2;
                break;
            }
        }
        else {
            // if we find a used port then we reset the counter
            counter = 0;
        }
    }
    cout << "PortKock: " << portNumber2 << " " << portNumber3 << " " << portNumber1 << endl;
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
    if(x.ports.size() != 3){
        return false;
    }
    string currentTime = getCurrTimestamp();
    //compare everything exept minutes and seconds
    for(int i = 0; i < currentTime.size()-5; i++) {
        if(currentTime[i] != x.connectTime[i]) {
            // we treat the port knock as incorrect if it doesnt happen within a minute
            return false;
        }
    }
    vector<int> temp = x.ports;
    // hardcoded port knock
    return (temp[0] == portNumber2 && temp[1] == portNumber3 && temp[2] == portNumber1);
}

void updateKnockMap(struct sockaddr_in clientAddress, map<string, Knock> &knockMap, int port) {
    // if a clients portvector contains three ports then erase the data
    if(knockMap.count(inet_ntoa(clientAddress.sin_addr)) > 0) {
        if(knockMap[inet_ntoa(clientAddress.sin_addr)].ports.size() >= 3) {
            knockMap.erase(inet_ntoa(clientAddress.sin_addr));
        }
    }
    // if a client exists in the map then we only update the portvector
    if(knockMap.count(inet_ntoa(clientAddress.sin_addr)) > 0) {
        knockMap[inet_ntoa(clientAddress.sin_addr)].ports.push_back(port);
    } // if a client does not exist int the map then we add him to the map
    else {
        pair<string, Knock> x;
        x.first = inet_ntoa(clientAddress.sin_addr);
        x.second.ports.push_back(port);
        x.second.connectTime = getCurrTimestamp();
        knockMap.insert(x);
    }
}

void checkCommandAndReact(int &newSocketFD1, struct sockaddr_in& clientAddress, map<string, clientInfo>& usersmap) {

    char buffer[512];

    string result = msgBuf;
    msgBuf = "";
    string theCommand;
    string theMessage;

    vector<string> splitter;

    istringstream iss(result);
    for(string result; iss >> result; )
        splitter.push_back(result);

    if(splitter.size() == 2) {
        theCommand = splitter[0];
        theRequest = splitter[1];

        splitter.clear();
    }
    else if(splitter.size() == 3) {
        theCommand = splitter[0];
        theRequest = splitter[1];
        theMessage = splitter[2];
    }
    else if(splitter.size() == 1){
        theCommand = splitter[0];
        theRequest = "";
        splitter.clear();
    }
    else {
        return;
    }
    cout<<"theCommand:." << theCommand<<"."<<endl;
    cout<<"theRequest:." << theRequest<<"."<<endl;
    cout<<"theMessage:." << theMessage<<"."<<endl;

    if(theCommand == "ID")
        theRequest = serverID;

    else if(theCommand == "CHANGEID") {
        serverID = provide_unique_id();
        theRequest = serverID;
    }

    else if(theCommand == "CONNECT" && theRequest != "") {
        // if a client has not connected before then we add him to a client map
        if(usersmap.count(theRequest) <= 0) {
            pair<string, clientInfo> x;
            x.first = theRequest;
            x.second.sockfd = newSocketFD1;
            x.second.clientAddress = clientAddress;
            usersmap.insert(x);
            msgBuf = "";
        }
    }
    else if(theCommand == "MSG" && theRequest != "" && theRequest != "--ALL" && theMessage != "") {
        msgUser = theRequest;

        if(usersmap.count(msgUser) <= 0) {
            theRequest = "No user found";
        }
        else {
            cout<< "Now sending message: "<< theMessage<< ", to user: "<< msgUser<<endl;
            int cliSockfd = usersmap[msgUser].sockfd;
            struct sockaddr_in cliAddr = usersmap[msgUser].clientAddress;
            char * stringToChar = new char[theMessage.length()+1];
            strcpy (stringToChar, theMessage.c_str());
            sendto(cliSockfd, stringToChar, 200, 0, (struct sockaddr*)&cliAddr, sizeof cliAddr);
        }
    }
    else if(theCommand == "MSG" && theRequest == "--ALL" && theMessage != "") {
        cout << "Now sending message: " << theMessage << ", to everyone"<<endl;
        map<string, clientInfo>::iterator it;
        for(it = usersmap.begin(); it != usersmap.end(); it++) {
            int cliSockfd = it->second.sockfd;
            struct sockaddr_in cliAddr = it->second.clientAddress;
            char * stringToChar = new char[theMessage.length()+1];
            strcpy (stringToChar, theMessage.c_str());
            sendto(cliSockfd, stringToChar, 200, 0, (struct sockaddr*)&cliAddr, sizeof cliAddr);
            memset(buffer, 0, sizeof(buffer));
            stringToChar = NULL;
            delete stringToChar;
        }
        return;
    }
    else if(theCommand == "LEAVE") {
        cout<<theRequest<<": disconnecting.."<<endl;
        usersmap.erase(theRequest);
        return;
    }
    else if(theCommand == "WHO") {
        map<string, clientInfo>::iterator it;
        cout<< "-----listing all users-----"<<endl;
        theRequest = "";

        for(it = usersmap.begin(); it != usersmap.end(); it++) {
            theRequest += it->first + ", ";
        }
        cout<<"users: "<< theRequest <<endl;

    }
    memset(buffer, 0, sizeof(buffer));
    char * stringToChar = new char[theRequest.length()+1];
    strcpy (stringToChar, theRequest.c_str());
    sendto(newSocketFD1, stringToChar, 200, 0, (struct sockaddr*)&clientAddress, sizeof clientAddress);
}


int main(int argc, char *argv[]) {

    if (argc < 3) {
        cout << "use: ./chatServer startPort endPort" << endl;
        exit(1);
    }
    int startPort = stoi(argv[1]);
    int endPort = stoi(argv[2]);

    if((startPort + 1) >= endPort) {
        cout << "StarPort must be higher than endPort" << endl << "and there must be atleast one port between them" << endl;
        exit(1);
    }

    map<string, Knock> knockMap;
    map<string, clientInfo> usersmap;

    int socketFD1 = 0; // main socket
    int socketFD2 = 0; // socket for port knocking
    int socketFD3 = 0; // socket for port knocking

    int bindSuccess = 0;
    int portConnectedTo = 0; // the port thie client is connected to

    int newSocketFD1 = 0; // socket created when a client is accepted

    // create set of file descritor sets
    fd_set activeFDSet;
    fd_set readFDSet;

    int returnValue = 0; // return value for the read() and write() calls; i.e. it contains the number of characters read or written.
    socklen_t clilen = 0; // stores the size of the address of the client. This is needed for the accept system call.

    struct sockaddr_in serverAddress1;
    struct sockaddr_in serverAddress2;
    struct sockaddr_in serverAddress3;
    struct sockaddr_in clientAddress;

    // a unique id provided for the server when it is first launched
    serverID = provide_unique_id();

    // create sockets
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

    // initializes serverAddress to zeros.
    bzero((char *) &serverAddress1, sizeof(serverAddress1));
    bzero((char *) &serverAddress2, sizeof(serverAddress2));
    bzero((char *) &serverAddress3, sizeof(serverAddress3));

    // code for address family, this is always set to AF_INET
    serverAddress1.sin_family = AF_INET;
    serverAddress2.sin_family = AF_INET;
    serverAddress3.sin_family = AF_INET;

    // this will always be the IP address of the machine on which the server is running
    serverAddress1.sin_addr.s_addr = INADDR_ANY;
    serverAddress2.sin_addr.s_addr = INADDR_ANY;
    serverAddress3.sin_addr.s_addr = INADDR_ANY;

    // finds three avaiable consecutive ports and assigns them
    findAvailablePorts(startPort, endPort);

    // the ports are set for the server which are chosen in the findAvailablePorts function
    serverAddress1.sin_port = htons(portNumber1);
    serverAddress2.sin_port = htons(portNumber2);
    serverAddress3.sin_port = htons(portNumber3);

    //serverAddress.sin_port = htons(8001);
    bindSocket(socketFD1, serverAddress1);
    bindSocket(socketFD2, serverAddress2);
    bindSocket(socketFD3, serverAddress3);

    /*bind(socketFD1, (struct sockaddr *) &serverAddress1, sizeof(serverAddress1));
    bind(socketFD2, (struct sockaddr *) &serverAddress2, sizeof(serverAddress2));
    bind(socketFD3, (struct sockaddr *) &serverAddress3, sizeof(serverAddress3));*/

    if(listen(socketFD1, 5) < 0) {
        error("socket1");
    }
    if(listen(socketFD2, 5) < 0) {
        error("socket2");
    }
    if(listen(socketFD3, 5) < 0) {
        error("socket3");
    }

    // Initialize the set of active sockets.
    FD_ZERO (&activeFDSet);
    FD_SET (socketFD1, &activeFDSet);
    FD_SET (socketFD2, &activeFDSet);
    FD_SET (socketFD3, &activeFDSet);

    while(1) {
        // Block untill input arrives on one or more of the active sockets
        readFDSet = activeFDSet;
        //selectFileDescritporSet(readFDSet);
        int selectValue = select(FD_SETSIZE, &readFDSet, NULL, NULL, NULL);
        if(selectValue < 0) error("Select");

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

                    fprintf (stderr,
                             "Server: connect from host %s, port %hd.\n",
                             inet_ntoa (clientAddress.sin_addr),
                             ntohs (clientAddress.sin_port));

                   // find the correct port depending on which socket the client is connected
                   // this port is then added to the port knocking vector
                   if(i == socketFD1)
                       portConnectedTo = portNumber1;
                   else if(i == socketFD2)
                       portConnectedTo = portNumber2;
                   else
                       portConnectedTo = portNumber3;

                   updateKnockMap(clientAddress, knockMap, portConnectedTo);
                   Knock temp = knockMap[inet_ntoa(clientAddress.sin_addr)];

                   if(correctKnock(temp)){
                       // add the file descritor from the accept to the active fd set
                       FD_SET (newSocketFD1, &activeFDSet);
                   }
                   else {
                       // close the clients connection because he has not completed a correct port knock
                       close (newSocketFD1);
                   }
                }
                else {
                    // read data from client
                    if (read_from_client (i) < 0) {
                        close (i);
                        FD_CLR (i, &activeFDSet);
                    }
                    // do a correct response depending on which command the client ran
                    checkCommandAndReact(i, clientAddress, usersmap);
                }
            }
        }
    }
    return 0;
}
