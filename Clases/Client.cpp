#include "Client.h"

Client::Client(string ip, int puerto)
{
    this->ip = ip;
    this->puerto = puerto;
}

SOCKET Client::configConexion()
{
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        throw std::runtime_error("Error al inicializar Winsock");
    }

    // Crear el socket TCP
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        WSACleanup();
        throw std::runtime_error("Error al crear el socket");
    }

    // Configurar la dirección del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Conectar al servidor
    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        close();
        throw std::runtime_error("Error al conectar al servidor");
    }

    return clientSocket;
}

void Client::close()
{
    closesocket(clientSocket);
    WSACleanup();
}
