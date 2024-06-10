#include "Tanque.h"
#include <functional>

using std::function;

bool colorCmpT(Color c1, Color c2)
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

Tanque::Tanque(Color color)
{
    this->color = color;
    setSpawn();
    pos.x = spawn.x;
    pos.y = spawn.y;
    last_pos.x = pos.x;
    last_pos.y = pos.y;
    vel.x = 0.f;
    vel.y = 0.f;
    pos.width = TANKW;
    pos.height = TANKH;
    should_del = false;
    disp_timer = 0;
    ret_b = false;
    acel_b = false;
    click_b = false;
    left_b = false;
    right_b = false;
    acel_c = 0;
    ret_c = 0;
    left_c = 0;
    right_c = 0;
    frame_izq = 1;
    frame_der = 1;
    area_total = 0;
    escudo_b = false;
    escudo_timer = TANK_ESC_CD;
    escudo_duracion = 0;
    in_obs = false;
    colision_obs = false;
    vida = 100;
}

void Tanque::draw(array<Texture2D, 3> asset_tanque)
{
    Rectangle source = {0.0, 0.0, (float)asset_tanque[0].width, (float)asset_tanque[0].height / 2.f};
    Rectangle dest = {back.vertice[1].x, back.vertice[1].y, (float)asset_tanque[0].width, (float)asset_tanque[0].height / 2.f};
    DrawTextureEx(asset_tanque[frame_der], {back.vertice[1].x, back.vertice[1].y}, direccion * RAD2DEG * -1, 1, WHITE);
    DrawTexturePro(asset_tanque[frame_izq], source, dest, {0, 0}, direccion * RAD2DEG * -1, WHITE);
    if (escudo_b)
    {
        Color transparent = color;
        transparent.a = 90;
        DrawCircle(pos.x + 10 * v_u_front.x, pos.y - 10 * v_u_front.y, TANK_ESC_RAD, transparent);
    }
}

void Tanque::update(list<Obstaculo> obstaculos)
{
    // input();
    calcularDireccion();
    vectorUnitario();
    animacion();
    movimiento();
    area();
    colisionObs(obstaculos);
    colisionBorde();
    calcularVelocidad();
    respawn();
}

void Tanque::animacion()
{
    if (acel_b)
    {
        if (acel_c++ >= 12)
        {
            frame_izq = frame_izq != 2 ? frame_izq + 1 : 0;
            frame_der = frame_izq;
            acel_c = 0;
        }
    }
    else
    {
        if (ret_b)
        {
            if (ret_c++ >= 12)
            {
                frame_izq = frame_izq != 0 ? frame_izq - 1 : 2;
                frame_der = frame_izq;
                ret_c = 0;
            }
        }
        else
        {
            if (left_b)
            {
                if (left_c++ >= 8)
                {
                    frame_izq = frame_izq != 0 ? frame_izq - 1 : 2;
                    frame_der = frame_der != 2 ? frame_der + 1 : 0;
                    left_c = 0;
                }
            }
            if (right_b)
            {
                if (right_c++ >= 8)
                {
                    frame_izq = frame_izq != 2 ? frame_izq + 1 : 0;
                    frame_der = frame_der != 0 ? frame_der - 1 : 2;
                    right_c = 0;
                }
            }
        }
    }
}

void Tanque::input()
{
    acel_b = IsKeyDown(KEY_W) ? true : false;
    ret_b = IsKeyDown(KEY_S) ? true : false;
    left_b = IsKeyDown(KEY_A) ? true : false;
    right_b = IsKeyDown(KEY_D) ? true : false;
    click_b = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? true : false;
    clickr_b = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ? true : false;
}

void Tanque::movimiento()
{
    if (!colision_obs)
    {
        last_pos.x = pos.x;
        last_pos.y = pos.y;
        pos.x += vel.x;
        pos.y += vel.y;
    }
}

void Tanque::calcularDireccion()
{
    if (left_b)
    {
        direccion += DEG2RAD * DELTA_DIR;
        direccion = direccion > 2 * PI ? direccion - 2 * PI : direccion;
    }
    if (right_b)
    {
        direccion -= DEG2RAD * DELTA_DIR;
        direccion = direccion < 0 ? direccion + 2 * PI : direccion;
    }
}

void Tanque::calcularVelocidad()
{
    if (acel_b)
    {
        vel.y = v_u_front.y * TANKV * -1;
        vel.x = v_u_front.x * TANKV;
    }
    else
    {
        if (ret_b)
        {
            vel.y = (v_u_front.y * TANKV) / 1.2;
            vel.x = (v_u_front.x * TANKV) / 1.2 * -1;
        }
        else
        {
            vel.x = 0;
            vel.y = 0;
        }
    }
}

void Tanque::area()
{
    Vector2 vertices[4];

    vertices[0].x = pos.x + (v_u_front.x * (pos.width / 2.f) - v_u_side.x * (pos.height / 2.f));
    vertices[0].y = pos.y - (v_u_front.y * (pos.width / 2.f) - v_u_side.y * (pos.height / 2.f));

    vertices[1].x = pos.x + (v_u_front.x * (pos.width / 2.f) + v_u_side.x * (pos.height / 2.f));
    vertices[1].y = pos.y - (v_u_front.y * (pos.width / 2.f) + v_u_side.y * (pos.height / 2.f));

    vertices[2].x = pos.x - (v_u_front.x * (pos.width / 2.f) + v_u_side.x * (pos.height / 2.f));
    vertices[2].y = pos.y + (v_u_front.y * (pos.width / 2.f) + v_u_side.y * (pos.height / 2.f));

    vertices[3].x = pos.x - (v_u_front.x * (pos.width / 2.f) - v_u_side.x * (pos.height / 2.f));
    vertices[3].y = pos.y + (v_u_front.y * (pos.width / 2.f) - v_u_side.y * (pos.height / 2.f));

    /*
    auto f1 = [](float dir, float angulo_0)
    { return dir - angulo_0; };

    auto f2 = [](float dir, float angulo_0)
    { return dir + angulo_0; };

    auto f3 = [](float dir, float angulo_0)
    { return dir - PI - angulo_0; };

    auto f4 = [](float dir, float angulo_0)
    { return dir - PI + angulo_0; };

    vertices[0] = calcularVertices(f1);
    vertices[1] = calcularVertices(f2);
    vertices[2] = calcularVertices(f3);
    vertices[3] = calcularVertices(f4);
    */

    Vector2 cent;
    cent.x = pos.x;
    cent.y = pos.y;

    front.vertice[0] = cent;
    front.vertice[1] = vertices[0];
    front.vertice[2] = vertices[1];

    left.vertice[0] = cent;
    left.vertice[1] = vertices[2];
    left.vertice[2] = vertices[0];

    back.vertice[0] = cent;
    back.vertice[1] = vertices[2];
    back.vertice[2] = vertices[3];

    right.vertice[0] = cent;
    right.vertice[1] = vertices[1];
    right.vertice[2] = vertices[3];
}

/*
Vector2 Tanque::calcularVertices(function<float(float, float)> f)
{
    Vector2 triangulo;
    float angulo_0;
    float angulo_r;
    angulo_0 = atan((pos.height / 2.f) / (pos.width / 2.f));
    angulo_0 = f(direccion, angulo_0);
    angulo_r = radianes(90) - angulo_0;
    triangulo.y = TANKDIAG * sin(angulo_0);
    triangulo.x = TANKDIAG * sin(angulo_r);

    Vector2 vertice;
    vertice.x = pos.x + triangulo.x;
    vertice.y = pos.y - triangulo.y;

    return vertice;
}
*/

void Tanque::vectorUnitario()
{
    float rad = direccion + PI * 1.5;

    v_u_front.y = sin(direccion);
    v_u_front.x = cos(direccion);

    v_u_side.y = sin(rad);
    v_u_side.x = cos(rad);
}

Vector2 Tanque::canon()
{
    Vector2 vertice;
    vertice.x = pos.x + (v_u_front.x * (pos.width / 2.f) + v_u_front.x * (pos.height / 2.f));
    vertice.y = pos.y - (v_u_front.y * (pos.width / 2.f) + v_u_front.y * (pos.height / 2.f));
    return vertice;
}

void Tanque::colisionBorde()
{
    int y, x;
    for (x = BORDE_LEFT; x < BORDE_RIGHT; x++)
    {
        if (CheckCollisionPointTriangle(Vector2{float(x), float(BORDE_UP)}, front.vertice[0], front.vertice[1], front.vertice[2]))
        {
            if (acel_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(x), float(BORDE_UP)}, back.vertice[0], back.vertice[1], back.vertice[2]))
        {
            if (ret_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(x), float(BORDE_DOWN)}, front.vertice[0], front.vertice[1], front.vertice[2]))
        {
            if (acel_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(x), float(BORDE_DOWN)}, back.vertice[0], back.vertice[1], back.vertice[2]))
        {
            if (ret_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
    }

    for (y = BORDE_UP; y < BORDE_DOWN; y++)
    {
        if (CheckCollisionPointTriangle(Vector2{float(BORDE_LEFT), float(y)}, front.vertice[0], front.vertice[1], front.vertice[2]))
        {
            if (acel_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(BORDE_LEFT), float(y)}, back.vertice[0], back.vertice[1], back.vertice[2]))
        {
            if (ret_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(BORDE_RIGHT), float(y)}, front.vertice[0], front.vertice[1], front.vertice[2]))
        {
            if (acel_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
        if (CheckCollisionPointTriangle(Vector2{float(BORDE_RIGHT), float(y)}, back.vertice[0], back.vertice[1], back.vertice[2]))
        {
            if (ret_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }
    }
}

void Tanque::colisionObs(list<Obstaculo> obstaculos)
{
    Rectangle rec;
    colision_obs = false;
    in_obs = false;
    for (auto &obs : obstaculos)
    {
        rec = obs.getRec();
        if (CheckCollisionPointRec(front.vertice[1], rec) || CheckCollisionPointRec(front.vertice[2], rec))
        {
            if (acel_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }

        if (CheckCollisionPointRec(back.vertice[1], rec) || CheckCollisionPointRec(back.vertice[2], rec))
        {
            if (ret_b)
            {
                pos.x = last_pos.x;
                pos.y = last_pos.y;
                colision_obs = true;
            }
        }

        if (CheckCollisionPointRec(canon(), rec))
        {
            in_obs = true;
        }
    }
}

Proyectil Tanque::disparar()
{
    disp_timer++;
    if (canDisp())
    {
        Proyectil nuevo(canon(), direccion, color);
        disp_timer = 0;
        return nuevo;
    }
    throw std::runtime_error("ataque en cd");
}

bool Tanque::operator==(const Tanque &t) const
{
    if (this == &t)
    {
        return true;
    }
    return false;
}

bool Tanque::colisionProyectiles(list<Proyectil> &proyectiles)
{
    if (!escudo_b)
    {
        area_total = areaTriangulo(back.vertice[1], back.vertice[2], front.vertice[1]) + areaTriangulo(front.vertice[1], front.vertice[2], back.vertice[2]);
        float area1;
        float area2;
        float area3;
        float area4;
        float area_proyect;
        for (auto &disp : proyectiles)
        {
            if (!colorCmpT(disp.getColor(), this->getColor()))
            {
                area1 = areaTriangulo(disp.getCentro(), back.vertice[1], front.vertice[1]);
                area2 = areaTriangulo(disp.getCentro(), front.vertice[1], front.vertice[2]);
                area3 = areaTriangulo(disp.getCentro(), front.vertice[2], back.vertice[2]);
                area4 = areaTriangulo(disp.getCentro(), back.vertice[1], back.vertice[2]);

                area_proyect = area1 + area2 + area3 + area4;
                if (int(area_total) == int(area_proyect))
                {
                    disp.should_del = true;
                    return true;
                }
                if (int(area_total + 1) == int(area_proyect))
                {
                    disp.should_del = true;
                    return true;
                }
                if (int(area_total - 1) == int(area_proyect))
                {
                    disp.should_del = true;
                    return true;
                }
            }
        }
    }
    else
    {
        Vector2 triangulo;
        float distancia;
        for (auto &disp : proyectiles)
        {
            if (!colorCmpT(disp.getColor(), this->getColor()))
            {
                triangulo.x = abs(disp.getCentro().x - pos.x);
                triangulo.y = abs(disp.getCentro().y - pos.y);

                distancia = sqrt(triangulo.x * triangulo.x + triangulo.y * triangulo.y);
                if (distancia < RAD_PYTL + TANK_ESC_RAD)
                    disp.should_del = true;
            }
        }
    }
    return false;
}

float Tanque::areaTriangulo(Vector2 vertice1, Vector2 vertice2, Vector2 vertice3)
{
    return abs((vertice1.x * (vertice2.y - vertice3.y) + vertice2.x * (vertice3.y - vertice1.y) + vertice3.x * (vertice1.y - vertice2.y)) / 2.0);
}

void Tanque::respawn()
{
    if (vida < 0)
    {

        pos.x = spawn.x;
        pos.y = spawn.y;
        last_pos.x = pos.x;
        last_pos.y = pos.y;
        vel.x = 0.f;
        vel.y = 0.f;
        vida = 100;
        setSpawn();
    }
}



bool Tanque::escudo()
{
    escudo_timer = escudo_timer > 0 ? escudo_timer - 1 : 0;
    escudo_duracion++;
    if (escudo_duracion > TANK_ESC_DUR)
    {
        escudo_b = 0;
    }

    if (clickr_b)
    {
        if (escudo_timer == 0)
        {
            escudo_b = true;
            escudo_timer = TANK_ESC_CD;
            escudo_duracion = 0;
            return true;
        }
    }
    return false;
}

void Tanque::drawVida()
{
    DrawRectangle(pos.x - CASILLA / 2, pos.y - CASILLA / 1.5, CASILLA * (getVida() / 100.0), 4, GREEN);
}

void Tanque::setSpawn()
{
    if (colorCmpT(color, RED))
    {
        spawn = {BORDE_LEFT + CASILLA, BORDE_UP + CASILLA};
        direccion = 0;
        return;
    }

    if (colorCmpT(color, YELLOW))
    {
        spawn = {(BORDE_RIGHT - BORDE_LEFT) / 2, BORDE_UP + CASILLA};
        direccion = PI * 1.5;
        return;
    }
    if (colorCmpT(color, BLUE))
    {
        spawn = {BORDE_RIGHT - CASILLA, BORDE_UP + CASILLA};
        direccion = PI;
        return;
    }

    if (colorCmpT(color, PURPLE))
    {
        spawn = {BORDE_LEFT + CASILLA, BORDE_DOWN - CASILLA};
        direccion = 0;
        return;
    }

    if (colorCmpT(color, BLACK))
    {
        spawn = {BORDE_LEFT + CASILLA, (BORDE_DOWN - BORDE_UP) / 2 + CASILLA};
        direccion = 0;
        return;
    }

    if (colorCmpT(color, DARKGRAY))
    {
        spawn = {BORDE_RIGHT - CASILLA, (BORDE_DOWN - BORDE_UP) / 2 + CASILLA};
        direccion = PI;
        return;
    }

    if (colorCmpT(color, GREEN))
    {
        spawn = {(BORDE_RIGHT - BORDE_LEFT) / 2, BORDE_DOWN - CASILLA};
        direccion = PI / 2;
        return;
    }

    if (colorCmpT(color, SKYBLUE))
    {
        spawn = {BORDE_RIGHT - CASILLA, BORDE_DOWN - CASILLA};
        direccion = PI;
        return;
    }
}

bool Tanque::canDisp()
{
    if (disp_timer > VEL_ATK)
    {
        if (click_b)
        {
            if (!in_obs)
            {
                return true;
            }
        }
    }
    return false;
}