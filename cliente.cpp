#define RAYLIB_NO_WINDOWS_CONFLICTS
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "C:\raylib\w64devkit\x86_64-w64-mingw32\include\raylib.h"
#include "Clases/Tanque.h"
#include "Clases/Proyectil.h"
#include "t.h"
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;
using std::string;
using std::thread;
using std::to_string;

void initMapa(bool map[int(RALTO)][int(RANCHO)]);
void window();
void drawMapa(bool map[int(RALTO)][int(RANCHO)]);
void input(int bits[]);

SOCKET clientSocket;

map<int, Tanque> tanques;
int cont_tanques = 0;

int main()
{
    // Inicializar Winsock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        std::cerr << "Error al inicializar Winsock" << std::endl;
        return 1;
    }

    // Crear el socket TCP
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error al crear el socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // Configurar la dirección del servidor
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());

    // Conectar al servidor
    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error al conectar al servidor" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    thread updateThread;

    char buffer[sizeof(Tanque)];
    string respuesta;
    int bytes;
    int i;
    auto it = tanques.begin();
    int bits[5];
    string input_buffer;
    int recibido = 0;
    int envio = 0;
    try
    {
        updateThread = thread(window);
    }
    catch (const std::exception &e)
    {
        std::cerr << "excepcion: " << e.what() << std::endl;
    }

    while (true)
    {
        bytes = recv(clientSocket, buffer, sizeof(Tanque), 0);
        if (bytes == 10)
        {
            cont_tanques = atoi(buffer);
            for (i = 0; i < cont_tanques; i++)
            {
                bytes = recv(clientSocket, buffer, sizeof(Tanque), 0);
                if (bytes == sizeof(Tanque))
                {
                    memcpy(&tanques[i], buffer, sizeof(Tanque));
                }
            }
        }

        input(bits);

        input_buffer = to_string(bits[0]) + to_string(bits[1]) + to_string(bits[2]) + to_string(bits[3]) + to_string(bits[4]);

        send(clientSocket, input_buffer.c_str(), 5, 0);
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

void window()
{
    SetTargetFPS(60);
    InitWindow(RANCHO, RALTO, "Raylib y Winsock Ejemplo");

    bool mapa[int(RALTO)][int(RANCHO)] = {false};
    //map<int, list<Proyectil>> map_proyectiles;
    //auto it_map_proyectil = map_proyectiles.begin();
    auto tanque = tanques.begin();
    char buffer[10];
    initMapa(mapa);
    int i;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        tanque = tanques.begin();
        i = 1;
        while (tanque != tanques.end())
        {
            (*tanque).second.draw();
            //map_proyectiles[i] = (*tanque).second.proyectiles;
            tanque++;
            i++;
        }

        drawMapa(mapa);
        DrawText(itoa(GetFPS(), buffer, 10), 10, 10, 40, WHITE);
        EndDrawing();

        //it_map_proyectil = map_proyectiles.begin();
        //while (it_map_proyectil != map_proyectiles.end())
        {
            //auto it_lista_proyectil = (*it_map_proyectil).second.begin();
            //while (it_lista_proyectil != (*it_map_proyectil).second.end())
            {
            //    (*it_lista_proyectil).draw();
            //    it_lista_proyectil++;
            }
            //it_map_proyectil++;
        }
    }
    CloseWindow();
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

void drawMapa(bool map[int(RALTO)][int(RANCHO)])
{
    int i, j;
    for (i = 0; i < RALTO; i++)
    {
        for (j = 0; j < RANCHO; j++)
        {
            if (map[i][j])
            {
                DrawPixel(j, i, WHITE);
            }
        }
    }
}

void input(int bits[])
{
    bits[0] = IsKeyDown(KEY_A) ? 1 : 0;
    bits[1] = IsKeyDown(KEY_W) ? 1 : 0;
    bits[2] = IsKeyDown(KEY_S) ? 1 : 0;
    bits[3] = IsKeyDown(KEY_D) ? 1 : 0;
    bits[4] = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1 : 0;
}