#include "t.h"
#include "Clases/Proyectil.h"
#include "Clases/Tanque.h"
#include "Clases/Client.h"
#include "Clases/Obstaculo.h"
#include "Clases/Zona.h"

int main()
{
    Client c(IP, PUERTO);
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    try
    {
        c.configConexion();
        clientSocket = c.getSocket();
        serverAddr = c.getServerAddr();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    char buffer[5000];
    char bit[1] = {'2'}; 

    // Configurar el socket en modo no bloqueante
    u_long mode = 1; // 1 para no bloqueante
    ioctlsocket(clientSocket, FIONBIO, &mode);

    while (1)
    {
   
        int bytes = sendto(clientSocket, bit, sizeof(bit), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

   
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &fromlen);
        if (bytesReceived > 0)
        {
            int cantidad = bytesReceived / sizeof(_Tanque);
            if (bytesReceived == cantidad * (int)sizeof(_Tanque))
            {
                _Tanque t[cantidad];
                memcpy(&t, buffer, bytesReceived);
                for (int i = 0; i < cantidad; i++)
                {
                    cout << "\nx: " << t[i].pos.x;
                    cout << "y: " << t[i].pos.y;
                    cout << "i: " << i;
                }
            }
        }
        else
        {
            if (bytesReceived < 0)
            {
                
            }
        }
    }
    return 0;
}