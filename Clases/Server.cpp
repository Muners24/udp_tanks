#include "Server.h"

Server::Server(int puerto)
{
    SERVER_PORT = puerto;
}
void Server::configServer()
{
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        throw std::runtime_error("Error al inicializar Winsock");
    }

    // Crear el socket TCP
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET)
    {
        WSACleanup();
        throw std::runtime_error("Error al crear el socket");
    }

    // Configurar la dirección del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección y puerto
    if (bind(listeningSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        closeServer();
        throw std::runtime_error("Error al enlazar el socket");
    }

    // Poner el socket en modo de escucha
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closeServer();
        throw std::runtime_error("Error al poner el socket en modo de escucha");
    }
}

SOCKET Server::conexionCliente()
{
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(listeningSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
    if (clientSocket == INVALID_SOCKET)
    {
        throw std::runtime_error("Error al aceptar la conexión\n");
    }
    return clientSocket;
}

void Server::closeServer()
{
    closesocket(listeningSocket);
    WSACleanup();
}