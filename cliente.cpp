#include "C:\raylib\w64devkit\x86_64-w64-mingw32\include\raylib.h"
#include "Clases/Proyectil.h"
#include "Clases/Tanque.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "t.h"

#pragma comment(lib, "Ws2_32.lib")

const std::string SERVER_IP = "20.169.80.74";
//3389?
const int SERVER_PORT = 12345;
using std::string;
using std::to_string;

void initMapa(bool map[int(RALTO)][int(RANCHO)]);
void drawMapa(bool map[int(RALTO)][int(RANCHO)]);
void input(int bits[]);
void configConexion();
void comunicacion();

map<int, Proyectil> proyectiles;
map<int, Tanque> tanques;
int cont_proyectiles = 0;
int cont_tanques = 0;
SOCKET clientSocket;

int main()
{
    try
    {
        configConexion();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    SetTargetFPS(60);
    InitWindow(RANCHO, RALTO, "Raylib y Winsock Ejemplo");

    bool mapa[int(RALTO)][int(RANCHO)] = {false};
    map<int, Tanque>::iterator tanque;
    map<int, Proyectil>::iterator disp_it;
    char buffer[10];
    initMapa(mapa);

    while (!WindowShouldClose())
    {
        comunicacion();
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(itoa(GetFPS(), buffer, 10), 10, 10, 40, WHITE);

        tanque = tanques.begin();
        while (tanque != tanques.end())
        {
            if(!(*tanque).second.should_del)
            {
                (*tanque).second.draw();
            }
            tanque++;
        }

        disp_it = proyectiles.begin();
        while (disp_it != proyectiles.end())
        {
            if (!(*disp_it).second.should_del)
            {
                (*disp_it).second.draw();
            }
            disp_it++;
        }
        drawMapa(mapa);
        EndDrawing();
        tanques.clear();
        proyectiles.clear();
    }

    closesocket(clientSocket);
    WSACleanup();
    CloseWindow();
    return 0;
}

void configConexion()
{
    WSADATA wsData;
    sockaddr_in serverAddr;

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
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());

    // Conectar al servidor
    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        closesocket(clientSocket);
        WSACleanup();
        throw std::runtime_error("Error al conectar al servidor");
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

void comunicacion()
{
    char buffer[sizeof(Tanque)];
    string input_buffer;
    int bits[5];
    int bytes;
    int i;

    // sendinput
    input(bits);
    input_buffer = to_string(bits[0]) + to_string(bits[1]) + to_string(bits[2]) + to_string(bits[3]) + to_string(bits[4]);
    send(clientSocket, input_buffer.c_str(), 6, 0);

    // recvTanques
    bytes = recv(clientSocket, buffer, 10, 0);
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

    // recvProyectiles
    bytes = recv(clientSocket, buffer, 32, 0);
    if (bytes == 32)
    {
        cont_proyectiles = atoi(buffer);
        for (i = 0; i < cont_proyectiles; i++)
        {
            bytes = recv(clientSocket, buffer, sizeof(Proyectil), 0);
            if (bytes == sizeof(Proyectil))
            {
                memcpy(&proyectiles[i], buffer, sizeof(Proyectil));
            }
        }
    }
}