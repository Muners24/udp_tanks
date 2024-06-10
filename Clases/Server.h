#include "..\t.h"

class Server
{
    SOCKET listeningSocket;
    WSADATA wsData;
    sockaddr_in serverAddr;
    int SERVER_PORT;
    
public:
    Server(){};
    Server(int puerto);
    void configServer();
    SOCKET conexionCliente();
    void closeServer();
};