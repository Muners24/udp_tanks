#include "t.h"
#include "Clases/Proyectil.h"
#include "Clases/Tanque.h"
#include "Clases/Client.h"
#include "Clases/Obstaculo.h"
#include "Clases/Zona.h"

#pragma comment(lib, "Ws2_32.lib")

void playEffect(Sound s, Vector2 destino, Vector2 origen);
array<Texture2D, 3> obtenerTanque(Color color);
void drawBoton(char str[], int x, int y, bool select);
bool colorCmpS(Color c1, Color c2);
void initTextreTanque();
void drawMiniMapa();
void drawEscudoCd();
void comunicacion();
void drawSuelo();
void drawBorde();
Color getColor();
void initZona();
void drawVida();
void inputT();
void input();
void view();
void menu();

int sel_color;
bool on_menu;
int op;

list<Proyectil> proyectiles;
list<Obstaculo> obstaculos;
queue<sound_msg> sonidos;
list<Tanque> tanques;
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

SOCKET clientSocket;
Tanque client_tank;
string ms;

Camera2D camara;

int bits[7];

int main()
{
    Client cliente("20.163.29.146", 12345);

    try
    {
        clientSocket = cliente.configConexion();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    vector<thread> hilos;
    on_menu = true;
    try
    {
        hilos.emplace_back(comunicacion);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    try
    {
        hilos.emplace_back(view);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    for (auto &hilo : hilos)
    {
        if (hilo.joinable())
        {
            hilo.join();
        }
    }

    cliente.close();
    return 0;
}

void comunicacion()
{
    Tanque temp_tanque;
    Proyectil temp_proyectil;
    Obstaculo temp_obstaculo;
    Zona temp_zona;
    char buffer[sizeof(Tanque)];
    string input_buffer;

    int bytes;
    int i;
    list<Proyectil> proyectiles_temp;
    list<Tanque> tanques_temp;
    // map<int, Proyectil> proyectiles_temp;
    // map<int, Tanque> tanques_temp;
    int cont_proyectiles = 0;
    int cont_tanques = 0;
    int cont_sounds = 0;
    int cont_obs = 0;

    while (on_menu)
    {
    }

    Color c = getColor();
    bytes = send(clientSocket, reinterpret_cast<const char *>(&c), sizeof(Color), 0);

    bytes = recv(clientSocket, buffer, 5, 0);
    if (bytes == 5)
    {
        cont_obs = atoi(buffer);
        mtx.lock();
        obstaculos.clear();
        for (i = 0; i < cont_obs; i++)
        {
            bytes = recv(clientSocket, buffer, sizeof(Obstaculo), 0);
            if (bytes == sizeof(Obstaculo))
            {
                memcpy(&temp_obstaculo, buffer, sizeof(Obstaculo));
                obstaculos.push_back(temp_obstaculo);
            }
        }
        mtx.unlock();
    }

    while (true)
    {
        auto inicio = std::chrono::high_resolution_clock::now();
        // sendinput
        inputT();
        input_buffer = to_string(bits[0]) + to_string(bits[1]) + to_string(bits[2]) + to_string(bits[3]) + to_string(bits[4]) + to_string(bits[5]);
        bytes = send(clientSocket, input_buffer.c_str(), 7, 0);

        // recvSound
        bytes = recv(clientSocket, buffer, 4, 0);
        if (bytes == 4)
        {
            cont_sounds = atoi(buffer);
            for (i = 0; i < cont_sounds; i++)
            {
                sound_msg temp_sonido;
                bytes = recv(clientSocket, buffer, sizeof(sound_msg), 0);
                if (bytes == sizeof(sound_msg))
                {
                    memcpy(&temp_sonido, buffer, sizeof(sound_msg));
                    if (temp_sonido.should_rep)
                        sonidos.push(temp_sonido);
                }
            }
        }

        // recvTanques
        bytes = recv(clientSocket, buffer, 10, 0);
        if (bytes == 10)
        {
            cont_tanques = atoi(buffer);
            tanques_temp.clear();
            for (i = 0; i < cont_tanques; i++)
            {
                if (i == 0)
                {
                    bytes = recv(clientSocket, buffer, sizeof(Tanque), 0);
                    if (bytes == sizeof(Tanque))
                    {
                        memcpy(&client_tank, buffer, sizeof(Tanque));
                        tanques_temp.push_back(client_tank);
                    }
                }
                else
                {
                    bytes = recv(clientSocket, buffer, sizeof(Tanque), 0);
                    if (bytes == sizeof(Tanque))
                    {
                        memcpy(&temp_tanque, buffer, sizeof(Tanque));
                        tanques_temp.push_back(temp_tanque);
                    }
                }
            }

            // mtx.lock();
            tanques = std::move(tanques_temp);
            // mtx.unlock();
        }

        // recvProyectiles
        bytes = recv(clientSocket, buffer, 32, 0);
        if (bytes == 32)
        {
            cont_proyectiles = atoi(buffer);
            proyectiles_temp.clear();
            for (i = 0; i < cont_proyectiles; i++)
            {

                bytes = recv(clientSocket, buffer, sizeof(Proyectil), 0);
                if (bytes == sizeof(Proyectil))
                {
                    memcpy(&temp_proyectil, buffer, sizeof(Proyectil));
                    proyectiles_temp.push_back(temp_proyectil);
                }
            }

            // mtx.lock();
            proyectiles = std::move(proyectiles_temp);
            // mtx.unlock();
        }

        // recvZonas
        for (i = 0; i < 25; i++)
        {
            bytes = recv(clientSocket, buffer, sizeof(Zona), 0);
            memcpy(&temp_zona, buffer, sizeof(Zona));
            zonas[i] = temp_zona;
        }

        auto fin = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
        ms = "MS:" + to_string(duracion.count());
    }
}

void view()
{
    SetTargetFPS(60);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "TANKS");
    SetWindowPosition(0, 10);

    InitAudioDevice();
    initZona();
    initTextreTanque();

    string ping;
    char buffer1[10];
    char buffer2[10];
    int i;
    bool listo = false;

    Sound mov = LoadSound("audio\\mov.wav");
    Sound danio = LoadSound("audio\\danio.wav");
    Sound disp = LoadSound("audio\\exp.wav");
    Sound shield = LoadSound("audio\\shield.wav");
    SetSoundVolume(mov, 0.3f);
    float actual_pitch = 1.0f;
    losa = LoadTexture("img\\losa.png");
    borde = LoadTexture("img\\borde.png");
    box = LoadTexture("img\\box.png");
    menu();

    camara.target.x = client_tank.getCentro().x;
    camara.target.y = client_tank.getCentro().y;
    camara.offset.x = GetScreenWidth() / 2;
    camara.offset.y = GetScreenHeight() / 2;
    camara.rotation = 0;
    camara.zoom = 0.8;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        BeginMode2D(camara);
        ClearBackground(DARKBLUE);
        drawSuelo();
        drawBorde();

        for (auto &obs : obstaculos)
        {
            obs.draw(box);
        }

        if (!IsSoundPlaying(mov))
        {
            PlaySound(mov);
        }

        if (bits[0] || bits[1] || bits[2] || bits[3])
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

        {
            mtx.lock();
            while (!sonidos.empty())
            {
                sound_msg s = sonidos.front();
                sonidos.pop();

                switch (s.id)
                {
                case DISP:
                    playEffect(disp, client_tank.getCentro(), s.origen);
                    break;
                case DANIO:
                    playEffect(danio, client_tank.getCentro(), s.origen);
                    break;
                case SHIELD:
                    playEffect(shield, client_tank.getCentro(), s.origen);
                    break;
                }
            }

            i = 0;
            for (auto &tanque : tanques)
            {
                tanque.draw(obtenerTanque(tanque.getColor()));
                if (i != 0)
                {
                    tanque.drawVida();
                }
                i++;
            }
            for (auto &disp : proyectiles)
            {
                disp.draw();
            }
            mtx.unlock();
        }

        

        mtx.lock();
        if (camara.target.x < client_tank.getCentro().x)
        {
            camara.target.x += 2;
        }
        if (camara.target.y < client_tank.getCentro().y)
        {
            camara.target.y += 2;
        }
        if (camara.target.x > client_tank.getCentro().x)
        {
            camara.target.x -= 2;
        }
        if (camara.target.y > client_tank.getCentro().y)
        {
            camara.target.y -= 2;
        }
        mtx.unlock();

        EndMode2D();
        for (auto &zona : zonas)
        {
            zona.getPrc(client_tank);
        }

        drawEscudoCd();
        strcpy(buffer2, "FPS: ");
        itoa(GetFPS(), buffer1, 10);
        strcat(buffer2, buffer1);
        DrawText(buffer2, 20, 30, 35, WHITE);
        drawVida();

        drawMiniMapa();
        EndDrawing();
        if (!listo)
        {
            camara.target.x = client_tank.getCentro().x;
            camara.target.y = client_tank.getCentro().y;
            camara.offset.x = GetScreenWidth() / 2;
            camara.offset.y = GetScreenHeight() / 2;
            camara.rotation = 0;
            camara.zoom = 0.8;
            listo = true;
        }
    }

    CloseWindow();
}

void inputT()
{
    bits[0] = IsKeyDown(KEY_A) ? 1 : 0;
    bits[1] = IsKeyDown(KEY_W) ? 1 : 0;
    bits[2] = IsKeyDown(KEY_S) ? 1 : 0;
    bits[3] = IsKeyDown(KEY_D) ? 1 : 0;
    bits[4] = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1 : 0;
    bits[5] = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ? 1 : 0;
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

void initTextreTanque()
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

void menu()
{
    Image img_fondo = LoadImage("img\\menu.png");
    // Obtiene la resolución de la pantalla
    int width = GetScreenWidth();
    // Redimensiona la imagen a las nuevas dimensiones calculadas
    ImageResize(&img_fondo, GetScreenWidth(), GetScreenHeight());

    Texture2D fondo = LoadTextureFromImage(img_fondo);
    Texture2D mouse = LoadTexture("img\\mouse.png");
    Texture2D wasd = LoadTexture("img\\wasd.png");
    op = 0;
    sel_color = 0;
    int actual;
    while (on_menu)
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(fondo, {0, 0}, 0, 1, Color{220, 220, 220, 255});
        actual = 100;
        drawBoton((char *)"TNKRD", width / 2.0, actual, false);
        actual += 150;
        if (op == 0)
            drawBoton((char *)"Jugar", width / 2.0, actual, true);
        else
            drawBoton((char *)"Jugar", width / 2.0, actual, false);
        actual += 100;
        if (op == 1)
        {
            drawBoton((char *)" <- Color ->", width / 2.0, actual, true);
            actual += 100;
            DrawRectangle(width / 2.0 - CASILLA - 4, actual - 4, CASILLA * 2 + 8, CASILLA * 2 + 8, BLACK);
            DrawRectangle(width / 2.0 - CASILLA, actual, CASILLA * 2, CASILLA * 2, getColor());
            actual += 100;
        }
        else
            drawBoton((char *)"Color", width / 2, actual, false);

        actual += 100;
        if (op == 2)
        {
            drawBoton((char *)"Controles", width / 2.0, actual, true);
            actual += 100;
            DrawRectangle(width / 2.0 - CASILLA * 3, actual, CASILLA * 6, CASILLA * 2.5, Color{220, 220, 220, 255});
            DrawTexture(wasd, width / 2.0 - CASILLA * 3, actual - 50, Color{220, 220, 220, 255});
            DrawTexture(mouse, width / 2.0 + CASILLA, actual + 40, Color{220, 220, 220, 255});
            actual += 150;
        }
        else
            drawBoton((char *)"Controles", width / 2.0, actual, false);

        actual += 100;
        if (op == 3)
            drawBoton((char *)"Salir", width / 2.0, actual, true);
        else
            drawBoton((char *)"Salir", width / 2.0, actual, false);

        EndDrawing();

        input();
    }
}

void drawBoton(char str[], int x, int y, bool select)
{
    int ancho;
    ancho = MeasureText(str, 70);
    Vector2 pos = {(float)x - ancho / 2 + 12, (float)y};
    if (select)
    {
        DrawRectangle(pos.x - 20, pos.y - 8, ancho + 10, 70, BLACK);
        DrawText(str, pos.x, pos.y, 60, Color{220, 220, 220, 255});
    }
    else
    {
        DrawRectangle(pos.x - 20, pos.y - 8, ancho + 10, 70, Color{220, 220, 220, 255});
        DrawText(str, pos.x, pos.y, 60, BLACK);
    }
}

void input()
{
    if (IsKeyPressed(KEY_W))
    {
        op = op > 1 ? op - 1 : 0;
    }
    if (IsKeyPressed(KEY_S))
    {
        op = op < 3 ? op + 1 : 3;
    }

    if (op == 1)
    {
        if (IsKeyPressed(KEY_A))
        {
            sel_color = sel_color > 1 ? sel_color - 1 : 7;
        }
        if (IsKeyPressed(KEY_D))
        {
            sel_color = sel_color < 7 ? sel_color + 1 : 0;
        }
    }

    if (op == 0)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            on_menu = false;
        }
    }
    if (op == 3)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            exit(0);
        }
    }
}

Color getColor()
{
    if (sel_color == 0)
        return RED;
    if (sel_color == 1)
        return YELLOW;
    if (sel_color == 2)
        return BLUE;
    if (sel_color == 3)
        return PURPLE;
    if (sel_color == 4)
        return GREEN;
    if (sel_color == 5)
        return SKYBLUE;
    if (sel_color == 6)
        return BLACK;
    if (sel_color == 7)
        return DARKGRAY;
    return BLACK;
}

void drawEscudoCd()
{

    int ancho = MeasureText("FPS: 60", 35);
    Color transparent = client_tank.getColor();
    string timer = to_string((client_tank.getEscudoTimer() * 1 / 60));
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
    DrawRectangle(vida.x, vida.y, vida.width, vida.height * client_tank.getVida() / 100, Color{6, 174, 5, 200});
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
    DrawRectangle(iniciox - 45, inicioy, 45, 20, Color{255, 255, 255, 150});
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
            DrawRectangle(iniciox + rec.x / CASILLA * 5, inicioy + rec.y / CASILLA * 5, rec.width / CASILLA * 5, rec.height / CASILLA * 5, Color{167, 85, 68, 150});
        }
        for (auto &tanque : tanques)
        {
            c = tanque.getColor();
            if (colorCmpS(c, client_tank.getColor()))
            {
                rec = tanque.getRec();
                c.a = 220;
                DrawRectangle(iniciox + (rec.x - rec.width / 2.0f) / CASILLA * 5, inicioy + (rec.y - rec.height / 2.0f) / CASILLA * 5, 5, 5, c);
            }
            else
            {
                for (auto &t : tanques)
                {
                    if (colorCmpS(t.getColor(),client_tank.getColor()))
                    {
                        Vector2 t1, t2;
                        t1 = tanque.getCentro();
                        t2 = t.getCentro();
                        float dist = sqrtf((t1.x - t2.x) * (t1.x - t2.x) + (t1.y - t2.y) * (t1.y - t2.y));
                        if (dist < 1000)
                        {
                            rec = tanque.getRec();
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
    for (int i = 0; i < 25; i++)
    {
        zonas.push_back(Zona());
    }
}