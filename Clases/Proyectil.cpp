#include "Proyectil.h"

_Proyectil Proyectil::toStruct()
{
    _Proyectil p;
    p.rebotes_cont = this->rebotes_cont;
    p.centro = this->centro;
    p.vel = this->vel;
    p.color = this->color;
    p.radio = this->radio;
    p.should_del = this->should_del;
    return p;
}

Proyectil::Proyectil(_Proyectil p)
{
    this->rebotes_cont = p.rebotes_cont;
    this->centro = p.centro;
    this->vel = p.vel;
    this->color = p.color;
    this->radio = p.radio;
    this->should_del = p.should_del;
}

Proyectil::Proyectil(Vector2 fuente, float direccion, Color color)
{
    this->color = color;
    rebotes_cont = 0;
    radio = RAD_PYTL;
    should_del = false;
    centro.x = fuente.x;
    centro.y = fuente.y;
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

void Proyectil::colisionObs(list<Obstaculo> obstaculos)
{
    for (auto &obs : obstaculos)
    {
        Rectangle rec = obs.getRec();
        if (CheckCollisionPointLine(centro, {rec.x, rec.y}, {rec.x + rec.width, rec.y}, 8))
        {
            vel.y *= -1;
            centro.y -= VEL_PYTL;
            rebotes_cont++;
            continue;
        }
        if (CheckCollisionPointLine(centro, {rec.x, rec.y + rec.height}, {rec.x + rec.width, rec.y + rec.height}, 8))
        {
            vel.y *= -1;
            centro.y += VEL_PYTL;
            rebotes_cont++;
            continue;
        }
        if (CheckCollisionPointLine(centro, {rec.x, rec.y}, {rec.x, rec.y + rec.height}, 8))
        {
            vel.x *= -1;
            centro.x -= VEL_PYTL;
            rebotes_cont++;
            continue;
        }
        if (CheckCollisionPointLine(centro, {rec.x + rec.width, rec.y}, {rec.x + rec.width, rec.y + rec.height}, 8))
        {
            vel.x *= -1;
            centro.x += VEL_PYTL;
            rebotes_cont++;
            continue;
        }
    }
}

void Proyectil::movimiento()
{
    centro.x += vel.x;
    centro.y += vel.y;
}

void Proyectil::update(list<Obstaculo> obstaculos)
{
    movimiento();
    colisionBorde();
    colisionObs(obstaculos);
    if (rebotes_cont > 4)
    {
        should_del = true;
    }
}

void Proyectil::draw()
{
    DrawCircle(centro.x, centro.y, radio, BLACK);
    DrawCircle(centro.x, centro.y, radio - 4, color);
}

bool Proyectil::operator==(const Proyectil &p) const
{
    if (this == &p)
    {
        return true;
    }
    return false;
}
