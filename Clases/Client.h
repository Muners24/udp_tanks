#include "..\t.h"

class Client
{
private:
    
    SOCKET clientSocket;
    WSADATA wsData;
    sockaddr_in serverAddr;
    string ip;
    int puerto;

public:
    int id;
    Client(){};
    Client(string ip,int puerto);
    SOCKET getSocket(){return clientSocket;};
    void configConexion();
    void close();
    sockaddr_in getServerAddr(){return serverAddr;};
    int getId(){return id;};
};