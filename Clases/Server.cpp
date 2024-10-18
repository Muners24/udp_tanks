#include "Server.h"
#include <stdexcept>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

Server::Server(int puerto)
{
    SERVER_PORT = puerto;
}

void Server::configServer()
{
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        std::cerr << "Error al inicializar Winsock." << std::endl;
        return;
    }

    // Crear el socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Error al crear el socket." << std::endl;
        WSACleanup();
        return;
    }

    // Configurar la dirección del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Aceptar conexiones desde cualquier dirección

    // Vincular el socket
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error al vincular el socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    //Activa no bloqueante
    //u_long mode;
    // Obtener el modo del socket
    //if (ioctlsocket(serverSocket, FIONBIO, &mode) != 0)
    //{
    //    perror("ioctlsocket falló");
    //}

    std::cout << "Servidor UDP configurado en el puerto " << SERVER_PORT << std::endl;
}

void Server::closeServer()
{
    closesocket(serverSocket);
    WSACleanup();
}