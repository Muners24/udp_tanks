#include "t.h"
#include "Clases/Proyectil.h"
#include "Clases/Tanque.h"
#include "Clases/Server.h"
#include "Clases/Obstaculo.h"
#include "Clases/SoundMsg.h"
#include "Clases/Zona.h"

#pragma comment(lib, "Ws2_32.lib")

const int SERVER_PORT = 12345;

void playEffect(Sound s, Vector2 destino, Vector2 origen);
bool CheckCollisionRecsss(Rectangle rect1, Rectangle rect2);
array<Texture2D, 3> obtenerTanque(Color color);
void comunicacionClient(SOCKET clientSocket);
bool colorCmpS(Color c1, Color c2);

void listener();
void sendProyectiles(sockaddr_in &to);
void sendObstaculos(sockaddr_in &to);
void sendTanques(sockaddr_in &to);
void sendSonidos(sockaddr_in &to);
void sendZonas(sockaddr_in &to);
void sendId(sockaddr_in &to);

void recvDiscon(sockaddr_in &from);
void recvColor(sockaddr_in &from);
void recvInput(sockaddr_in &from);

void initObstaculo();
void drawMiniMapa();
void drawEscudoCd();
void updateJuego();
void initTexture();
void initCamara();
void initSonido();
void drawBorde();
void drawSuelo();
void drawVida();
void initZona();

queue<SoundMsg> cola_sonidos;
list<Proyectil> proyectiles;
list<Obstaculo> obstaculos;
list<Tanque *> tanques;
list<SoundMsg> sonidos;
list<int> clients_id;
vector<Zona> zonas;
mutex mtx;

array<Texture2D, 3> t_red;
array<Texture2D, 3> t_blue;
array<Texture2D, 3> t_purple;
array<Texture2D, 3> t_cian;
array<Texture2D, 3> t_black_r;
array<Texture2D, 3> t_black_d;
array<Texture2D, 3> t_green;
array<Texture2D, 3> t_yellow;
Texture2D losa;
Texture2D borde;
Texture2D box;

Sound danio;
Sound disp;
Sound mov;
Sound shield;

Tanque *tanque_server;
SOCKET listeningSocket;

string ms;

Camera2D camara;

int current_client;
Server server;

int cont_tanque;

int main()
{
    cont_tanque = 0;

    server = Server(PUERTO);

    try
    {
        server.configServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    thread threadJuego;
    threadJuego = thread(updateJuego);

    tanque_server = new Tanque(BLUE, cont_tanque++);
    tanques.push_back(tanque_server);

    while (true)
    {
        listener();
    }

    server.closeServer();
    return 0;
}

void updateJuego()
{
    SetTargetFPS(60);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "server");
    SetWindowPosition(0, 10);

    InitAudioDevice();
    initObstaculo();
    initTexture();
    initCamara();
    initSonido();
    initZona();

    auto disp_it = proyectiles.begin();
    auto it = tanques.begin();
    auto s = sonidos.begin();

    float actual_pitch = 1.0f;
    char buffer1[10];
    char buffer2[10];

    while (!WindowShouldClose())
    // while (true)
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
        BeginDrawing();
        BeginMode2D(camara);
        ClearBackground(BLACK);
        drawSuelo();
        drawBorde();
        mtx.lock();
        s = sonidos.begin();
        while (s != sonidos.end())
        {
            if (s->readyToRemove())
            {
                s = sonidos.erase(s);
            }
            else
            {
                s++;
            }
        }
        mtx.unlock();

        if (!IsSoundPlaying(mov))
        {
            PlaySound(mov);
        }

        if ((tanque_server)->wasd.up || (tanque_server)->wasd.down || (tanque_server)->wasd.left || (tanque_server)->wasd.right)
        {
            if (actual_pitch != 1.1f)
            {
                actual_pitch = 1.1f;
                SetSoundPitch(mov, actual_pitch);
            }
        }
        else
        {
            if (actual_pitch != 1.0f)
            {
                actual_pitch = 1.0f;
                SetSoundPitch(mov, actual_pitch);
            }
        }

        for (auto &obs : obstaculos)
        {
            obs.draw(box);
        }

        mtx.lock();
        it = tanques.begin();
        while (it != tanques.end())
        {
            if (!(*it)->should_del)
            {

                (*it)->draw(obtenerTanque((*it)->getColor()));

                if ((*it) != tanque_server)
                {
                    (*it)->drawVida();
                }

                (*it)->update(obstaculos);

                try
                {
                    Proyectil nuevo = (*it)->disparar();
                    proyectiles.push_front(nuevo);
                    playEffect(disp, tanque_server->getCentro(), (*it)->getCentro());
                    sonidos.push_back(SoundMsg(DISP, clients_id, (*it)->getCentro()));
                }
                catch (const std::exception &e)
                {
                }

                if ((*it)->colisionProyectiles(proyectiles))
                {
                    (*it)->danio();
                    playEffect(danio, tanque_server->getCentro(), (*it)->getCentro());
                    sonidos.push_back(SoundMsg(DANIO, clients_id, (*it)->getCentro()));
                }

                if ((*it)->escudo())
                {
                    playEffect(shield, tanque_server->getCentro(), (*it)->getCentro());
                    sonidos.push_back(SoundMsg(SHIELD, clients_id, (*it)->getCentro()));
                }

                it++;
            }
            else
            {
                it = tanques.erase(it);
            }
        }

        disp_it = proyectiles.begin();
        while (disp_it != proyectiles.end())
        {
            if (!(*disp_it).should_del)
            {
                (*disp_it).draw();
                (*disp_it).update(obstaculos);
                disp_it++;
            }
            else
            {
                disp_it = proyectiles.erase(disp_it);
            }
        }
        mtx.unlock();

        EndMode2D();

        for (auto &zona : zonas)
        {
            zona.update(tanques);
            zona.getPrc(*tanque_server);
        }
        drawEscudoCd();
        strcpy(buffer2, "FPS: ");
        itoa(GetFPS(), buffer1, 10);
        strcat(buffer2, buffer1);
        DrawText(buffer2, 20, 30, 35, WHITE);
        drawVida();

        tanque_server->input();
        if (camara.target.x < tanque_server->getCentro().x)
        {
            camara.target.x += 2;
        }
        if (camara.target.y < tanque_server->getCentro().y)
        {
            camara.target.y += 2;
        }
        if (camara.target.x > tanque_server->getCentro().x)
        {
            camara.target.x -= 2;
        }
        if (camara.target.y > tanque_server->getCentro().y)
        {
            camara.target.y -= 2;
        }
        drawMiniMapa();
        EndDrawing();
    }
    server.closeServer();
    CloseWindow();
    exit(0);
}

bool colorCmpS(Color c1, Color c2)
{
    if (c1.a == c2.a)
    {
        if (c1.r == c2.r)
        {
            if (c1.g == c2.g)
            {
                if (c1.b == c2.b)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void initTexture()
{
    t_red[0] = LoadTexture("img\\T_RED0.png");
    t_red[1] = LoadTexture("img\\T_RED1.png");
    t_red[2] = LoadTexture("img\\T_RED2.png");

    t_blue[0] = LoadTexture("img\\T_BLUE0.png");
    t_blue[1] = LoadTexture("img\\T_BLUE1.png");
    t_blue[2] = LoadTexture("img\\T_BLUE2.png");

    t_purple[0] = LoadTexture("img\\T_PURPLE0.png");
    t_purple[1] = LoadTexture("img\\T_PURPLE1.png");
    t_purple[2] = LoadTexture("img\\T_PURPLE2.png");

    t_cian[0] = LoadTexture("img\\T_CIAN0.png");
    t_cian[1] = LoadTexture("img\\T_CIAN1.png");
    t_cian[2] = LoadTexture("img\\T_CIAN2.png");

    t_black_d[0] = LoadTexture("img\\T_BLACKD0.png");
    t_black_d[1] = LoadTexture("img\\T_BLACKD1.png");
    t_black_d[2] = LoadTexture("img\\T_BLACKD2.png");

    t_black_r[0] = LoadTexture("img\\T_BLACKR0.png");
    t_black_r[1] = LoadTexture("img\\T_BLACKR1.png");
    t_black_r[2] = LoadTexture("img\\T_BLACKR2.png");

    t_green[0] = LoadTexture("img\\T_GREEN0.png");
    t_green[1] = LoadTexture("img\\T_GREEN1.png");
    t_green[2] = LoadTexture("img\\T_GREEN2.png");

    t_yellow[0] = LoadTexture("img\\T_YELLOW0.png");
    t_yellow[1] = LoadTexture("img\\T_YELLOW1.png");
    t_yellow[2] = LoadTexture("img\\T_YELLOW2.png");

    t_cian[0] = LoadTexture("img\\T_CIAN0.png");
    t_cian[1] = LoadTexture("img\\T_CIAN1.png");
    t_cian[2] = LoadTexture("img\\T_CIAN2.png");

    losa = LoadTexture("img\\losa.png");
    borde = LoadTexture("img\\borde.png");
    box = LoadTexture("img\\box.png");
}

void initObstaculo()
{
    srand(time(NULL));
    int x, y;
    for (y = BORDE_UP + CASILLA * 2; y < BORDE_DOWN - CASILLA * 4; y += CASILLA)
    {
        for (x = BORDE_LEFT + CASILLA * 2; x < BORDE_RIGHT - CASILLA * 4; x += CASILLA)
        {
            if ((rand() % 12) == 1)
            {
                bool colision = false;
                Obstaculo obs({(float)x, (float)y});
                for (auto obstaculo : obstaculos)
                {
                    if (CheckCollisionRecsss(obstaculo.getRec(), obs.getRec()))
                    {
                        colision = true;
                    }
                }
                if (!colision)
                {
                    obstaculos.emplace_back(obs);
                }
            }
        }
    }
}

array<Texture2D, 3> obtenerTanque(Color color)
{
    if (colorCmpS(color, RED))
        return t_red;
    if (colorCmpS(color, YELLOW))
        return t_yellow;
    if (colorCmpS(color, BLUE))
        return t_blue;
    if (colorCmpS(color, PURPLE))
        return t_purple;
    if (colorCmpS(color, BLACK))
        return t_black_d;
    if (colorCmpS(color, DARKGRAY))
        return t_black_r;
    if (colorCmpS(color, GREEN))
        return t_green;
    if (colorCmpS(color, SKYBLUE))
        return t_cian;
    return t_red;
}

void drawSuelo()
{
    int x, y;
    for (y = -RALTO; y < BORDE_DOWN + RALTO; y += 160)
    {
        for (x = -RANCHO; x < BORDE_RIGHT + RANCHO; x += 160)
        {
            DrawTexture(losa, x, y, WHITE);
        }
    }
}

void drawBorde()
{
    int x, y;
    for (x = BORDE_LEFT - CASILLA; x <= BORDE_RIGHT; x += CASILLA)
    {
        DrawTexture(borde, x, BORDE_UP - CASILLA, WHITE);
        DrawTexture(borde, x, BORDE_DOWN, WHITE);
    }
    for (y = BORDE_UP - CASILLA; y <= BORDE_DOWN; y += CASILLA)
    {
        DrawTexture(borde, BORDE_LEFT - CASILLA, y, WHITE);
        DrawTexture(borde, BORDE_RIGHT, y, WHITE);
    }
}

void drawEscudoCd()
{
    int ancho = MeasureText("FPS: 60", 35);
    Color transparent = tanque_server->getColor();
    string timer = to_string(int(tanque_server->getEscudoTimer()/ 60.0f)+1);
    if (tanque_server->getEscudoTimer() != 0)
    {
        transparent = Color{25, 25, 25, 90};
    }
    else
    {
        timer = '0';
    }
    transparent.a = 90;

    DrawCircle(20 + ancho / 2, 120, TANK_ESC_RAD / 1.5, transparent);
    DrawText(timer.c_str(), 20 + ancho / 2 - MeasureText(timer.c_str(), 40) / 2.0, 102, 40, WHITE);
}

void drawVida()
{
    int ancho = MeasureText("FPS: 60", 35);
    float iniciox = 20 + ancho / 2 - 20;
    float inicioy = 180;
    Rectangle vida;
    vida.x = iniciox;
    vida.y = inicioy;
    vida.height = 300;
    vida.width = 40;
    DrawRectangle(vida.x, vida.y, vida.width, vida.height * tanque_server->getVida() / 100, Color{6, 174, 5, 200});
    DrawRectangle(iniciox, inicioy, 2, 300, BLACK);
    DrawRectangle(iniciox, inicioy, 40, 2, BLACK);
    DrawRectangle(iniciox, inicioy + 298, 40, 2, BLACK);
    DrawRectangle(iniciox + 38, inicioy, 2, 300, BLACK);
}

void playEffect(Sound s, Vector2 destino, Vector2 origen)
{
    Sound ss = LoadSoundAlias(s);
    float max_dist = 1500;
    float max_vol = 0.8f;
    float distance = sqrtf(powf(origen.x - destino.x, 2) + powf(origen.y - destino.y, 2));
    float volume = max_vol - (distance / max_dist);
    volume = fmaxf(volume, 0.0f);
    float balance = (((origen.x - destino.x) / max_dist) / 2.0f * -1 + 0.5);
    SetSoundVolume(ss, volume);
    SetSoundPan(ss, balance);

    PlaySound(ss);
}

void drawMiniMapa()
{
    static bool open = true;
    float ancho = (float)GetScreenWidth();
    float alto = (float)GetScreenHeight();
    float iniciox = ancho - 5 - COLUMNAS * 5;
    float inicioy = 15;
    DrawRectangle(iniciox - 45, inicioy, 45, 20, Color{255, 255, 255, 110});
    DrawText("tab", iniciox - 38, inicioy, 20, Color{0, 0, 0, 150});
    if (IsKeyPressed(KEY_TAB))
    {
        open = !open;
    }

    if (open)
    {
        int x, y;
        int cuady = FILAS * 5 + inicioy;
        int cuadx = COLUMNAS * 5 + iniciox;
        int incremento = 50;
        int i = 0;
        Rectangle rec;
        Color c;
        for (auto &zona : zonas)
        {
            rec = zona.getRec();
            c = zona.getColor();
            DrawRectangle(iniciox + rec.x / CASILLA * 5, inicioy + rec.y / CASILLA * 5, rec.width / CASILLA * 5, rec.height / CASILLA * 5, c);
        }

        for (auto &obs : obstaculos)
        {
            rec = obs.getRec();
            DrawRectangle(iniciox + rec.x / CASILLA * 5, inicioy + rec.y / CASILLA * 5, rec.width / CASILLA * 5, rec.height / CASILLA * 5, Color{177, 95, 78, 150});
        }

        for (auto &tanque : tanques)
        {
            c = tanque->getColor();
            if (colorCmpS(c, tanque_server->getColor()))
            {
                rec = tanque->getRec();
                c.a = 220;
                DrawRectangle(iniciox + (rec.x - rec.width / 2.0f) / CASILLA * 5, inicioy + (rec.y - rec.height / 2.0f) / CASILLA * 5, 5, 5, c);
            }
            else
            {
                for (auto &t : tanques)
                {
                    if (colorCmpS(t->getColor(), tanque_server->getColor()))
                    {
                        Vector2 t1, t2;
                        t1 = tanque->getCentro();
                        t2 = t->getCentro();
                        float dist = sqrtf((t1.x - t2.x) * (t1.x - t2.x) + (t1.y - t2.y) * (t1.y - t2.y));
                        if (dist < 1000)
                        {
                            rec = tanque->getRec();
                            c.a = 220;
                            DrawRectangle(iniciox + (rec.x - rec.width / 2.0f) / CASILLA * 5, inicioy + (rec.y - rec.height / 2.0f) / CASILLA * 5, 5, 5, c);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void initZona()
{
    int x, y;
    for (y = 0; y < BORDE_RIGHT; y += CASILLA * 10)
    {
        for (x = 0; x < BORDE_DOWN; x += CASILLA * 10)
        {
            zonas.push_back(Zona(x, y));
        }
    }
}

void initSonido()
{
    danio = LoadSound("audio\\danio.wav");
    disp = LoadSound("audio\\exp.wav");
    shield = LoadSound("audio\\shield.wav");

    mov = LoadSound("audio\\mov.wav");
    SetSoundVolume(mov, 0.15f);
}

void initCamara()
{
    camara.target.x = tanque_server->getCentro().x;
    camara.target.y = tanque_server->getCentro().y;
    camara.offset.x = GetScreenWidth() / 2;
    camara.offset.y = GetScreenHeight() / 2;
    camara.rotation = 0;
    camara.zoom = 0.8;
}

bool CheckCollisionRecsss(Rectangle rect1, Rectangle rect2)
{
    // Verificar colisión en el eje X
    bool collisionX = rect1.x < rect2.x + rect2.width && rect1.x + rect1.width > rect2.x;

    // Verificar colisión en el eje Y
    bool collisionY = rect1.y < rect2.y + rect2.height && rect1.y + rect1.height > rect2.y;

    // Si hay colisión en ambos ejes, los rectángulos se solapan
    return collisionX && collisionY;
}

void listener()
{
    char buffer[255];
    sockaddr_in from;
    socklen_t fromlen = sizeof(from);
 
    int bytesRecv = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (sockaddr *)&from, &fromlen);
    if (bytesRecv > 0)
    {
        if (bytesRecv == 1)
        {
            printf("%c", buffer[0]);
            switch (buffer[0])
            {
            case SEND_OBSTACULOS:
                sendObstaculos(from);
                break;
            case SEND_PROYECTILES:
                sendProyectiles(from);
                break;
            case SEND_SONIDOS:
                sendSonidos(from);
                break;
            case SEND_TANQUES:
                sendTanques(from);
                break;
            case SEND_ZONAS:
                sendZonas(from);
                break;
            case SEND_ID:
                sendId(from);
                break;
            case SEND_INPUT:
                recvInput(from);
                break;
            case SEND_COLOR:
                recvColor(from);
                break;
            case SEND_DISCONNECT:
                recvDiscon(from);
                break;
            default:
                break;
            }
        }
    }
    else
    {
        if (bytesRecv < 0)
        {
        }
    }
    buffer[0] = '\0';
}

void sendTanques(sockaddr_in &to)
{
    int len = tanques.size();
    _Tanque pack_tanque[len];

    int i = 0;
    for (auto &tanque : tanques)
    {
        pack_tanque[i] = tanque->toStruct();
        i++;
    }
    if (i == len)
    {
        int bytes = -1;
        while (bytes < 0 || bytes != len * (int)sizeof(_Tanque))
        {
            bytes = sendto(server.getSocket(), (char *)pack_tanque, sizeof(pack_tanque), 0, (struct sockaddr *)&to, sizeof(to));
        }
        //printf("\nbytes enviados tanques: %d\n", bytes);
    }
}

void sendProyectiles(sockaddr_in &to)
{
    int len = proyectiles.size();
    if (len > 0)
    {
        _Proyectil pack_proyectil[len];
        int i = 0;
        for (auto &proyectil : proyectiles)
        {
            pack_proyectil[i] = proyectil.toStruct();
            i++;
        }
        if (i == len)
        {
            int bytes = -1;
            while (bytes < 0 || bytes != len * (int)sizeof(_Proyectil))
            {
                bytes = sendto(server.getSocket(), (char *)pack_proyectil, sizeof(pack_proyectil), 0, (struct sockaddr *)&to, sizeof(to));
            }
            //printf("\nbytes enviados proyectiles: %d\n", bytes);
        }
    }
    else
    {
        int bytes = -1;
        int no_send = 0;
        while (bytes < 0 || bytes != (int)sizeof(int))
        {
            bytes = sendto(server.getSocket(), reinterpret_cast<char *>(&no_send), sizeof(int), 0, (struct sockaddr *)&to, sizeof(to));
        }
    }
}

void sendZonas(sockaddr_in &to)
{
    int len = zonas.size();
    _Zona pack_zona[len];

    int i = 0;
    for (auto &zona : zonas)
    {
        pack_zona[i] = zona.toStruct();
        i++;
    }

    if (i == len)
    {
        int bytes = -1;
        while (bytes < 0 || bytes != len * (int)sizeof(_Zona))
        {
            bytes = sendto(server.getSocket(), reinterpret_cast<char *>(pack_zona), sizeof(pack_zona), 0, (struct sockaddr *)&to, sizeof(to));
        }
    }
}

void sendObstaculos(sockaddr_in &to)
{
    int len = obstaculos.size();
    _Obstaculo pack_obstaculo[len];

    int i = 0;
    for (auto &obstaculo : obstaculos)
    {
        pack_obstaculo[i] = obstaculo.toStruct();
        i++;
    }

    if (i == len)
    {
        int bytes = -1;
        while (bytes < 0 || bytes != len * sizeof(_Obstaculo))
        {
            bytes = sendto(server.getSocket(), reinterpret_cast<char *>(pack_obstaculo), sizeof(pack_obstaculo), 0, (struct sockaddr *)&to, sizeof(to));
        }
    }
}

void sendSonidos(sockaddr_in &to)
{
    char buffer[50];
    sockaddr_in newfrom;
    socklen_t newfromlen = sizeof(newfrom);
    int len = sonidos.size();
    int bytes = -1;
    while (bytes < 0 || bytes != (int)sizeof(int))
    {
        bytes = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (sockaddr *)&newfrom, &newfromlen);
    }

    int id;
    memcpy(&id, buffer, sizeof(int));
    _SoundMsg pack_sonido_temp[len];
    int i = 0;
    int k = 0;
    int count_should_play = 0;
    for (auto &sonido : sonidos)
    {
        if (sonido.shouldPlay(id))
        {
            _SoundMsg temp = sonido.toStruct();
            pack_sonido_temp[count_should_play++] = temp;
        }
        i++;
    }

    if (len > 0 && count_should_play > 0)
    {
        if (i == len)
        {
            _SoundMsg pack_sonido[count_should_play];
            for (int j = 0; j < count_should_play; j++)
            {
                pack_sonido[j] = pack_sonido_temp[j];
            }
            bytes = -1;
            while (bytes < 0 || bytes != (int)sizeof(pack_sonido))
            {
                bytes = sendto(server.getSocket(), reinterpret_cast<char *>(pack_sonido), sizeof(pack_sonido), 0, (struct sockaddr *)&to, sizeof(to));
            }
        }
    }
    else
    {
        bytes = -1;
        while (bytes < 0 || bytes != (int)sizeof(int))
        {
            bytes = sendto(server.getSocket(), reinterpret_cast<char *>(&len), sizeof(int), 0, (struct sockaddr *)&to, sizeof(to));
        }
    }
}

void sendId(sockaddr_in &to)
{
    int id = cont_tanque++;
    int bytes = -1;
    int id_asign = -1;
    char buffer[500];
    sockaddr_in from;
    socklen_t fromlen;
    while (bytes < 0)
    {
        bytes = sendto(server.getSocket(), reinterpret_cast<char *>(&id), sizeof(int), 0, (struct sockaddr *)&to, sizeof(to));
    }

    while (id_asign != id)
    {
        bytes = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &fromlen);
        if (bytes == (int)sizeof(int))
            memcpy(&id_asign, buffer, sizeof(int));
    }
    clients_id.push_back(id_asign);
}

void recvColor(sockaddr_in &from)
{
    char buffer[5000];
    sockaddr_in newfrom;
    socklen_t newfromlen = sizeof(newfrom);
    int bytes;
    bytes = -1;
    while (bytes < 0 || bytes != (int)sizeof(_PeticionTanque))
    {
        bytes = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (struct sockaddr *)&newfrom, &newfromlen);
    }

    _PeticionTanque pt;
    memcpy(&pt, buffer, sizeof(_PeticionTanque));
    tanques.push_back(new Tanque(pt.color, pt.id));
}

void recvInput(sockaddr_in &from)
{
    char buffer[500];
    sockaddr_in newfrom;
    socklen_t fromlen;

    int bytes = -1;
    while (bytes < 0)
    {
        bytes = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (sockaddr *)&from, &fromlen);
    }
    int bits[7];

    memcpy(&bits, buffer, 7 * (int)sizeof(int));

    for (auto &tanque : tanques)
    {
        if (tanque->id == bits[6])
        {
            tanque->wasd.left = bits[0] == 1 ? true : false;
            tanque->wasd.up = bits[1] == 1 ? true : false;
            tanque->wasd.down = bits[2] == 1 ? true : false;
            tanque->wasd.right = bits[3] == 1 ? true : false;
            tanque->click_b = bits[4] == 1 ? true : false;
            tanque->clickr_b = bits[5] == 1 ? true : false;
        }
    }
}

void recvDiscon(sockaddr_in &from)
{
    char buffer[500];
    sockaddr_in newfrom;
    socklen_t fromlen;

    int bytes = -1;
    while (bytes < 0 || bytes != (int)sizeof(int))
    {
        printf("\nesperando id a desconectar\n");
        bytes = recvfrom(server.getSocket(), buffer, sizeof(buffer), 0, (sockaddr *)&from, &fromlen);
    }

    int id;
    memcpy(&id, buffer, sizeof(int));
    printf("\nrecibio id a desconectar: %d \n", id);

    auto it = tanques.begin();
    while (it != tanques.end())
    {
        printf("iterator id: %d idclient: %d", (*it)->id, id);
        if ((*it)->id == id)
        {
            tanques.erase(it);
        }
        it++;
    }

    printf("\ntanque eliminado\n");
}