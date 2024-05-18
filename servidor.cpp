#include "C:\raylib\w64devkit\x86_64-w64-mingw32\include\raylib.h"
#include "Clases/Proyectil.h"
#include "Clases/Tanque.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <list>
#include <map>
#include "t.h"

#pragma comment(lib, "Ws2_32.lib")

const int SERVER_PORT = 12345;

using std::thread;
using std::list;
using std::map;

void comunicacionClient(SOCKET clientSocket, int numeroCliente);
void initMapa(bool map[int(RALTO)][int(RANCHO)]);
SOCKET conexionCliente();
void configServer();
void updateJuego();

map<int, Tanque> tanques;
list<Proyectil> proyectiles;

SOCKET listeningSocket;

int main()
{
    try
    {
        configServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    

    list<thread> clientThreads;
    thread entradaCliente;
    SOCKET clientSocket;
    thread threadJuego;
    int i = 0;

    threadJuego = thread(updateJuego);

    while (true)
    {
        try
        {
            clientSocket = conexionCliente();
            tanques[++i] = Tanque(SPAWN1, RED);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            clientThreads.emplace_back(comunicacionClient, clientSocket, i);
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

void configServer()
{
    WSADATA wsData;
    sockaddr_in serverAddr;

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
        closesocket(listeningSocket);
        WSACleanup();
        throw std::runtime_error("Error al enlazar el socket");
    }

    // Poner el socket en modo de escucha
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(listeningSocket);
        WSACleanup();
        throw std::runtime_error("Error al poner el socket en modo de escucha");
    }
}

SOCKET conexionCliente()
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

void comunicacionClient(SOCKET clientSocket, int numeroCliente)
{
    auto proyectil = proyectiles.begin();
    char buffer[sizeof(Tanque)];
    auto it = tanques.begin();
    int bits[5];
    int bytes;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        // comunicacion input
        bytes = recv(clientSocket, buffer, 6, 0);
        if (bytes == 6)
        {
            it = tanques.begin();
            while (it != tanques.end())
            {
                if ((*it).first == numeroCliente)
                {
                    (*it).second.left_b = buffer[0] == '1' ? true : false;
                    (*it).second.acel_b = buffer[1] == '1' ? true : false;
                    (*it).second.ret_b = buffer[2] == '1' ? true : false;
                    (*it).second.right_b = buffer[3] == '1' ? true : false;
                    (*it).second.click_b = buffer[4] == '1' ? true : false;
                }
                it++;
            }
        }

        // comunicacion tanques
        itoa(tanques.size(), buffer, 10);
        send(clientSocket, buffer, 10, 0);
        it = tanques.begin();
        while (it != tanques.end())
        {
            const Tanque &tanque = (*it).second;
            send(clientSocket, reinterpret_cast<const char *>(&tanque), sizeof(Tanque), 0);
            it++;
        }

        // comunicacion proyectiles
        itoa(proyectiles.size(), buffer, 10);
        send(clientSocket, buffer, 32, 0);
        proyectil = proyectiles.begin();
        while (proyectil != proyectiles.end())
        {
            const Proyectil &proyec = (*proyectil);
            send(clientSocket, reinterpret_cast<const char *>(&proyec), sizeof(Proyectil), 0);
            proyectil++;
        }
    }

    closesocket(clientSocket);
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

void updateJuego()
{
    SetTargetFPS(60);
    InitWindow(RANCHO, RALTO, "servidor");
    auto it = tanques.begin();
    auto disp_it = proyectiles.begin();
    bool mapa[int(RALTO)][int(RANCHO)] = {false};
    initMapa(mapa);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        it = tanques.begin();
        while (it != tanques.end())
        {
            if ((*it).second.click_b)
            {
                try
                {
                    Proyectil nuevo = (*it).second.disparar();
                    proyectiles.push_front(nuevo);
                }
                catch (const std::exception &e){}
            }
            (*it).second.update(mapa);
            (*it).second.draw();
            it++;
        }

        disp_it = proyectiles.begin();
        while (disp_it != proyectiles.end())
        {
            if (!(*disp_it).should_del)
            {
                (*disp_it).draw();
                (*disp_it).update();
                disp_it++;
            }
            else
            {
                disp_it = proyectiles.erase(disp_it);
            }
        }
        EndDrawing();
    }

    CloseWindow();
}