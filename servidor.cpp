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
bool reconectar(SOCKET &sock);

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
Server servidor(SERVER_PORT);

int main()
{

    try
    {
        servidor.configServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    current_client = 0;

    list<thread> clientThreads;
    thread entradaCliente;
    SOCKET clientSocket;
    thread threadJuego;

    threadJuego = thread(updateJuego);
    tanque_server = new Tanque(BLACK);
    tanques.push_back(tanque_server);

    while (true)
    {

        try
        {
            clientSocket = servidor.conexionCliente();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            clientThreads.emplace_back(comunicacionClient, clientSocket);
        }
        catch (const std::exception &e)
        {
            std::cerr << "excepcion: " << e.what() << std::endl;
        }
    }

    servidor.closeServer();
    return 0;
}

void comunicacionClient(SOCKET clientSocket)
{
    auto proyectil = proyectiles.begin();
    auto obs = obstaculos.begin();
    auto it = tanques.begin();
    auto s = sonidos.begin();
    auto z = zonas.begin();

    list<Proyectil> proyectiles_temp;
    list<Tanque *> tanques_temp;
    list<SoundMsg> sonidos_temp;

    Color c;
    Tanque *t_ptr = NULL;

    int id = current_client++;
    clients_id.push_back(id);

    char buffer[sizeof(Tanque)];
    bool conectado = true;
    int bytes;

    bytes = recv(clientSocket, buffer, sizeof(Color), 0);
    if (bytes == sizeof(Color))
    {
        memcpy(&c, buffer, sizeof(Color));
        t_ptr = new Tanque(c);
        tanques.push_back(t_ptr);
    }

    itoa(obstaculos.size(), buffer, 10);
    bytes = send(clientSocket, buffer, 5, 0);
    obs = obstaculos.begin();
    while (obs != obstaculos.end())
    {
        const Obstaculo &proyec = (*obs);
        bytes = send(clientSocket, reinterpret_cast<const char *>(&proyec), sizeof(Obstaculo), 0);
        obs++;
    }

    while (conectado)
    {
        // comunicacion input
        bytes = recv(clientSocket, buffer, 7, 0);
        if (bytes == 7)
        {
            it = tanques.begin();
            while (it != tanques.end())
            {
                if ((*it) == t_ptr)
                {
                    (*it)->left_b = buffer[0] == '1' ? true : false;
                    (*it)->acel_b = buffer[1] == '1' ? true : false;
                    (*it)->ret_b = buffer[2] == '1' ? true : false;
                    (*it)->right_b = buffer[3] == '1' ? true : false;
                    (*it)->click_b = buffer[4] == '1' ? true : false;
                    (*it)->clickr_b = buffer[5] == '1' ? true : false;
                }
                it++;
            }
        }

        // comunicacion sonidos
        itoa(sonidos.size(), buffer, 10);
        bytes = send(clientSocket, buffer, 4, 0);
        s = sonidos.begin();
        while (s != sonidos.end())
        {
            const sound_msg &sonido_temp = (sound_msg){(*s).getId(), (*s).getOrg(), (*s).shouldPlay(id)};
            send(clientSocket, reinterpret_cast<const char *>(&sonido_temp), sizeof(sound_msg), 0);
            s++;
        }

        // comunicacion tanques
        tanques_temp = tanques;
        itoa(tanques_temp.size(), buffer, 10);
        bytes = send(clientSocket, buffer, 10, 0);
        bytes = send(clientSocket, reinterpret_cast<const char *>(&(*t_ptr)), sizeof(Tanque), 0);
        it = tanques_temp.begin();
        while (it != tanques_temp.end())
        {
            const Tanque *tanque = (*it);
            if (&(*tanque) != &(*t_ptr))
            {
                bytes = send(clientSocket, reinterpret_cast<const char *>(&(*tanque)), sizeof(Tanque), 0);
            }
            it++;
        }

        // comunicacion proyectiles
        proyectiles_temp = proyectiles;
        itoa(proyectiles_temp.size(), buffer, 10);
        bytes = send(clientSocket, buffer, 32, 0);
        proyectil = proyectiles_temp.begin();
        while (proyectil != proyectiles_temp.end())
        {
            const Proyectil &proyec = (*proyectil);
            bytes = send(clientSocket, reinterpret_cast<const char *>(&proyec), sizeof(Proyectil), 0);
            proyectil++;
        }

        // comunicacion zonas
        z = zonas.begin();
        while (z != zonas.end())
        {
            const Zona &zona = (*z);
            bytes = send(clientSocket, reinterpret_cast<const char *>(&zona), sizeof(Zona), 0);
            z++;
        }
        if (bytes < 0)
        {
            conectado = false;
        }
    }

    closesocket(clientSocket);
    t_ptr->should_del = true;
    clients_id.remove(id);
}

void updateJuego()
{
    SetTargetFPS(60);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Servidor");
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
            if ((*s).readyToRemove())
            {
                s = sonidos.erase(s);
            }
            else
            {
                s++;
            }
        }
        while (!cola_sonidos.empty())
        {
            sonidos.push_back(cola_sonidos.front());
            cola_sonidos.pop();
        }
        mtx.unlock();

        if (!IsSoundPlaying(mov))
        {
            PlaySound(mov);
        }

        if ((tanque_server)->acel_b || (tanque_server)->ret_b || (tanque_server)->left_b || (tanque_server)->right_b)
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
                    cola_sonidos.push(SoundMsg(DISP, clients_id, (*it)->getCentro()));
                }
                catch (const std::exception &e)
                {
                }

                if ((*it)->colisionProyectiles(proyectiles))
                {
                    (*it)->danio();
                    playEffect(danio, tanque_server->getCentro(), (*it)->getCentro());
                    cola_sonidos.push(SoundMsg(DANIO, clients_id, (*it)->getCentro()));
                }

                if ((*it)->escudo())
                {
                    playEffect(shield, tanque_server->getCentro(), (*it)->getCentro());
                    cola_sonidos.push(SoundMsg(SHIELD, clients_id, (*it)->getCentro()));
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
    string timer = to_string(int(tanque_server->getEscudoTimer() / 60.0f));
    if (timer != "0")
    {
        transparent = Color{25, 25, 25, 90};
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
    SetSoundVolume(mov, 0.3f);
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