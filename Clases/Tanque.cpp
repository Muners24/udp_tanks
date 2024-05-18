#include "Tanque.h"
#include <functional>

using std::function;

Tanque::Tanque(Vector2 spawn, Color color)
{
    pos.x = spawn.x;
    pos.y = spawn.y;
    last_pos.x = pos.x;
    last_pos.y = pos.y;
    pos.width = TANKW;
    pos.height = TANKH;
    direccion = 0;
    should_del = false;
    this->color = color;
    disp_timer = 0;
    vel.x = 0.f;
    vel.y = 0.f;
    ret_b = false;
    acel_b = false;
    click_b = false;
    left_b = false;
    right_b = false;
}

void Tanque::draw()
{
    DrawRectanglePro(pos, {float(pos.width / 2.0), float(pos.height / 2.0)}, direccion*RAD2DEG * -1, color);
    // struct
    // DrawRectangleRec(pos, BLUE);
    // triangulos hitbox
    DrawTriangle(front.vertice[0], front.vertice[1], front.vertice[2], GREEN);
    // DrawTriangle(left.vertice[0], left.vertice[1], left.vertice[2], GREEN);
    // DrawTriangle(back.vertice[0], back.vertice[1], back.vertice[2], GREEN);
    // DrawTriangle(right.vertice[0], right.vertice[1], right.vertice[2], GREEN);
}

void Tanque::calcularDireccion()
{
    if (left_b)
    {
        direccion += DEG2RAD*DELTA_DIR;
        direccion = direccion > 2 * PI ? direccion - 2 * PI : direccion;
    }
    if (right_b)
    {
        direccion -= DEG2RAD*DELTA_DIR;
        direccion = direccion < 0 ? direccion + 2 * PI : direccion;
    }
}

void Tanque::update(bool map[int(RALTO)][int(RANCHO)])
{
    //input();
    calcularDireccion();
    vectorUnitario();
    movimiento();
    area();
    colisionBorde(map);
    area();
    calcularVelocidad();
    disp_timer++;
}

/*
void Tanque::input()
{
    acel_b = IsKeyDown(tecla_up) ? true : false;
    ret_b = IsKeyDown(tecla_down) ? true : false;
    left_b = IsKeyDown(tecla_left) ? true : false;
    right_b = IsKeyDown(tecla_right) ? true : false;
    click_b = IsKeyDown(tecla_atk) ? true : false;
}
*/
void Tanque::movimiento()
{
    last_pos.x = pos.x;
    last_pos.y = pos.y;
    pos.x += vel.x;
    pos.y += vel.y;
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
            vel.y = (v_u_front.y * TANKV) / 1.3;
            vel.x = (v_u_front.x * TANKV) / 1.3 * -1;
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

    vertices[0].x = pos.x + (v_u_front.x * (pos.width / 2.f) + v_u_side.x * (pos.height / 2.f));
    vertices[0].y = pos.y - (v_u_front.y * (pos.width / 2.f) + v_u_side.y * (pos.height / 2.f));

    vertices[1].x = pos.x + (v_u_front.x * (pos.width / 2.f) - v_u_side.x * (pos.height / 2.f));
    vertices[1].y = pos.y - (v_u_front.y * (pos.width / 2.f) - v_u_side.y * (pos.height / 2.f));

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
    left.vertice[1] = vertices[1];
    left.vertice[2] = vertices[2];

    back.vertice[0] = cent;
    back.vertice[1] = vertices[2];
    back.vertice[2] = vertices[3];

    right.vertice[0] = cent;
    right.vertice[1] = vertices[3];
    right.vertice[2] = vertices[0];
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

void Tanque::colisionBorde(bool map[int(RALTO)][int(RANCHO)])
{
    int y, x;

    for (x = 0; x < RANCHO; x++)
    {
        for (y = 0; y < RALTO; y++)
        {
            if (map[y][x])
            {
                if (CheckCollisionPointTriangle(Vector2{float(x), float(y)}, front.vertice[0], front.vertice[1], front.vertice[2]))
                {
                    if(!ret_b)
                    {
                        pos.x = last_pos.x;
                        pos.y = last_pos.y;
                    }
                }
                if (CheckCollisionPointTriangle(Vector2{float(x), float(y)}, back.vertice[0], back.vertice[1], back.vertice[2]))
                {
                    if(!acel_b)
                    {
                        pos.x = last_pos.x;
                        pos.y = last_pos.y;
                    }
                }
            }
        }
    }
}

Proyectil Tanque::disparar()
{
    disp_timer++;

    if (disp_timer > VEL_ATK)
    {
        if (click_b)
        {
            Proyectil nuevo(canon(), direccion, color);
            disp_timer = 0;
            return nuevo;
        }
    }
    throw std::runtime_error("ataque en cd");
}

bool Tanque::operator==(const Tanque &t) const
{
    if(this == &t)
    {
        return true;
    }
    return false;
}