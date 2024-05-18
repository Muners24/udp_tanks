#include "Proyectil.h"

Proyectil::Proyectil(Vector2 fuente,float direccion,Color color)
{
    Vector2 vect_uni;
    this->color = color;
    rebotes_cont = 0;
    radio = RAD_PYTL;
    should_del = false;
    centro.x=fuente.x;
    centro.y=fuente.y;
    vel.y = sin(direccion) * VEL_PYTL * -1;
    vel.x = cos(direccion) * VEL_PYTL;
}

void Proyectil::colisionBorde()
{
    Vector2 l = left();
    if (l.x < BORDE_LEFT)
    {
        vel.x *= -1;
        centro.x = BORDE_LEFT + radio;
        rebotes_cont++;
    }
    Vector2 r = right();
    if (r.x > BORDE_RIGHT)
    {
        vel.x *= -1;
        centro.x = BORDE_RIGHT - radio;
        rebotes_cont++;
    }
    Vector2 u = up();
    if (u.y < BORDE_UP)
    {
        vel.y *= -1;
        centro.y = BORDE_UP + radio;
        rebotes_cont++;
    }
    Vector2 d = down();
    if (d.y > BORDE_DOWN)
    {
        vel.y *= -1;
        centro.y = BORDE_DOWN - radio;
        rebotes_cont++;
    }
}

void Proyectil::movimiento()
{
    centro.x += vel.x;
    centro.y += vel.y;
}

void Proyectil::update()
{
    movimiento();
    colisionBorde();
    if(rebotes_cont > 4)
    {
        should_del = true;
    }
}

void Proyectil::draw()
{
    DrawCircle(centro.x,centro.y,radio,color);
}

bool Proyectil::operator==(const Proyectil& p) const
{
    if(centro.x == p.centro.x)
    {
        if(centro.y == p.centro.y)
        {
            if(vel.x == p.vel.x)
            {
                if(vel.y == p.vel.y)
                {
                    return true;
                }
            }
        }
    }
    return false;
}