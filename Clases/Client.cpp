#include "Client.h"

Client::Client(string ip, int puerto)
{
    this->ip = ip;
    this->puerto = puerto;
    this->id = -1;
}

void Client::configConexion()
{
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        throw std::runtime_error("Error al inicializar Winsock");
    }

    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        WSACleanup();
        throw std::runtime_error("Error al crear el socket");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
}

void Client::close()
{
    closesocket(clientSocket);
    WSACleanup();
}
