#include "..\t.h"

class Client
{
    SOCKET clientSocket;
    WSADATA wsData;
    sockaddr_in serverAddr;
    string ip;
    int puerto;

public:
    Client(){};
    Client(string ip, int puerto);
    SOCKET configConexion();
    void close();
};