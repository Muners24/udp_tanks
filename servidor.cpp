#define RAYLIB_NO_WINDOWS_CONFLICTS
#include <iostream>
#include "C:\raylib\w64devkit\x86_64-w64-mingw32\include\raylib.h"
#include "Clases/Tanque.h"
#include "Clases/Proyectil.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "t.h"

#pragma comment(lib, "Ws2_32.lib")

const int SERVER_PORT = 12345;

using std::list;
using std::map;
using std::mutex;
using std::thread;

void HandleClient(SOCKET clientSocket, int numeroCliente);
void updateJuego();
void initMapa(bool map[int(RALTO)][int(RANCHO)]);

map<int, Tanque> tanques;
SOCKET listeningSocket;
mutex mtx;

int main()
{
    SetTargetFPS(60);
    // Inicializar Winsock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        std::cerr << "Error al inicializar Winsock" << std::endl;
        return 1;
    }

    // Crear el socket TCP
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET)
    {
        std::cerr << "Error al crear el socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // Configurar la dirección del servidor
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección y puerto
    if (bind(listeningSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error al enlazar el socket" << std::endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    // Poner el socket en modo de escucha
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Error al poner el socket en modo de escucha" << std::endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    list<thread> clientThreads;
    thread entradaCliente;
    thread threadJuego;
    SOCKET clientSocket;

    int i = 0;

    threadJuego = thread(updateJuego);
    
    while (true)
    {
        cout << tanques.size();
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        clientSocket = accept(listeningSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Error al aceptar la conexión" << std::endl;
            closesocket(listeningSocket);
            WSACleanup();
            return 1;
        }
        else
        {
            tanques[++i] = Tanque(SPAWN1, RED);
        }

        try
        {
            clientThreads.emplace_back(HandleClient, clientSocket, i);
        }
        catch (const std::exception &e)
        {
            std::cerr << "excepcion: " << e.what() << std::endl;
        }
    }

    closesocket(listeningSocket);
    WSACleanup();
    return 0;
}

void HandleClient(SOCKET clientSocket, int numeroCliente)
{
    SetTargetFPS(60);
    bool isRunning = true;
    int tanques_cont;
    char buffer[sizeof(tanques)];
    int i;
    auto it = tanques.begin();
    int cont_tanques;
    int bytes;
    int bits[5];
    int envio = 0;
    int recibido = 0;
    while (isRunning)
    {
        itoa(tanques.size(),buffer,10);
        send(clientSocket, buffer, 10, 0);
        it = tanques.begin();
        while (it != tanques.end())
        {
            const Tanque& tanque = (*it).second;
            send(clientSocket, reinterpret_cast<const char*>(&tanque), sizeof(Tanque), 0);
            it++;
        }

        
        bytes = recv(clientSocket,buffer,5,0);
        if(bytes == 5)
        {
            printf("%s\n",buffer);
            it = tanques.begin();
            while (it != tanques.end())
            {
                if((*it).first == numeroCliente)
                {
                    (*it).second.left_b = buffer[0] == '1' ? true : false;
                    (*it).second.acel_b = buffer[1] == '1' ? true: false;
                    (*it).second.ret_b = buffer[2] == '1' ? true: false;
                    (*it).second.right_b = buffer[3] == '1' ? true: false;
                    (*it).second.click_b = buffer[4] == '1' ? true: false;
                }
                it++;
            }
        }
    }

    closesocket(clientSocket);
}

void updateJuego()
{
    auto it = tanques.begin();
    bool mapa[int(RALTO)][int(RANCHO)] = {false};
    initMapa(mapa);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        it = tanques.begin();
        while(it != tanques.end())
        {
            (*it).second.update(mapa);
            it++;
        }
    }
}

void initMapa(bool map[int(RALTO)][int(RANCHO)])
{
    int y, x;
    for (x = BORDE_LEFT; x < BORDE_RIGHT; x++)
    {
        map[int(BORDE_UP)][x] = true;
        map[int(BORDE_DOWN)][x] = true;
    }
    for (y = BORDE_UP; y < BORDE_DOWN; y++)
    {
        map[y][int(BORDE_LEFT)] = true;
        map[y][int(BORDE_RIGHT)] = true;
    }
}