#include "..\t.h"

class Server
{
    addrinfo conf;
    SOCKET serverSocket;
    WSADATA wsData;
    sockaddr_in serverAddr;
    int SERVER_PORT;
    
public:

    Server(){};
    Server(int puerto);
    void configServer();
    SOCKET getSocket(){ return serverSocket;};
    void closeServer();
};