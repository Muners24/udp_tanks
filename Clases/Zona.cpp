#include "Zona.h"

_Zona Zona::toStruct()
{
    _Zona z;
    z.pos = pos;
    z.color = color;
    z.anime_b = anime_b;
    for(int i=0;i<9;i++)
    {
        z.timer[i] = timer[i];
    }
    return z;
}

Zona::Zona(_Zona &z)
{
    this->pos = z.pos;
    this->color = z.color;
    this->anime_b = z.anime_b;
    for(int i=0;i<9;i++)
    {
        this->timer[i] = z.timer[i];
    }
}
bool colorCmpZ(Color c1, Color c2)
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
    return false;
}


Color getColorOnArray(int key)
{
    switch (key)
    {
    case 0:
        return RED;
        break;
    case 1:
        return YELLOW;
        break;
    case 2:
        return BLUE;
        break;
    case 3:
        return PURPLE;
        break;
    case 4:
        return GREEN;
        break;
    case 5:
        return SKYBLUE;
        break;
    case 6:
        return BLACK;
        break;
    case 7:
        return DARKGRAY;
        break;
    }
    return WHITE;
}

int getIntFromColor(Color c)
{
    if (colorCmpZ(c, RED))
        return 0;
    if (colorCmpZ(c, YELLOW))
        return 1;
    if (colorCmpZ(c, BLUE))
        return 2;
    if (colorCmpZ(c, PURPLE))
        return 3;
    if (colorCmpZ(c, GREEN))
        return 4;
    if (colorCmpZ(c, SKYBLUE))
        return 5;
    if (colorCmpZ(c, BLACK))
        return 6;
    if (colorCmpZ(c, DARKGRAY))
        return 7;
    if (colorCmpZ(c, WHITE))
        return 8;
    return 0;
}

Zona::Zona(int x, int y)
{
    pos.x = (float)x;
    pos.y = (float)y;
    pos.width = CASILLA * 10;
    pos.height = CASILLA * 10;
    color = WHITE;
    color.a = 110;
    anime_b = false;

    for (int i = 0; i < 9; i++)
    {
        timer[i] = 0;
    }
}

void Zona::colisionsTanques(list<Tanque *> tanques)
{
    int cantidad = 0;
    int i;
    Color c;
    for (auto &t : tanques)
    {
        if (colisionTanque(*t))
        {
            if (!colorCmpZ(c, t->getColor()))
            {
                cantidad++;
            }
            c = t->getColor();
        }
    }

    if (cantidad == 0)
    {
        if (!colorCmpZ(color, WHITE))
        {
            timer[getIntFromColor(color)]++;
        }

        for (i = 0; i < 9; i++)
        {
            if (!colorCmpZ(color, getColorOnArray(i)))
            {
                if (timer[i] > 0)
                {
                    timer[i]--;
                    anime_b = true;
                }
            }
        }
    }
    else if (cantidad == 1)
    {
        bool suma = true;

        for (i = 0; i < 9; i++)
        {
            if (!colorCmpZ(c, getColorOnArray(i)))
            {

                if (timer[i] > 0)
                {
                    timer[i]--;
                    anime_b = true;
                }

                if (timer[i] != 0)
                {
                    suma = false;
                }
            }
        }

        if (suma)
        {
            timer[getIntFromColor(c)]++;
            anime_b = true;
        }
    }
    else if (cantidad > 1)
    {
    }
}

void Zona::transform()
{
    for (int i = 0; i < 9; i++)
    {
        if (timer[i] > ZONA_TTT)
        {
            Color c = getColorOnArray(i);
            c.a = 110;
            color = c;
            timer[i] = ZONA_TTT;
        }
    }

    if (!colorCmpZ(color, WHITE))
    {
        if (timer[getIntFromColor(color)] == 0)
        {
            Color c = WHITE;
            c.a = 110;
            color = c;
        }
    }
}

bool Zona::colisionTanque(Tanque t)
{
    if (CheckCollisionPointRec(t.getCentro(), pos))
        return true;
    return false;
}

void Zona::update(list<Tanque *> tanques)
{
    colisionsTanques(tanques);
    animacion();
    transform();
}

void Zona::animacion()
{
    if (anime_b)
    {
        color.a += 2;
        if (color.a > 110)
        {
            color.a = 0;
        }
    }
    else
    {
        color.a = 110;
    }

    anime_b = false;

    for (int i = 0; i < 9; i++)
    {
        if (timer[i] != 0 && timer[i] != 100)
        {
            anime_b = true;
            break;
        }
    }
}

void Zona::getPrc(Tanque t)
{
    Color c = t.getColor();

    if (CheckCollisionPointRec(t.getCentro(), pos))
    {
        if (colorCmpZ(color, WHITE))
        {
            int max_i = 0;
            int max_timer = 0;
            for (int i = 0; i < 9; i++)
            {
                if (timer[i] > max_timer)
                {
                    max_timer = timer[i];
                    max_i = i;
                }
            }

            if (colorCmpZ(c, getColorOnArray(max_i)))
            {
                drawPrc(c, GREEN);
            }
            else
            {
                drawPrc(getColorOnArray(max_i), RED);
            }
        }
        else
        {
            if (colorCmpZ(c, color))
            {
                drawPrc(c, GREEN);
            }
            else
            {
                drawPrc(color, RED);
            }
        }
    }
}

void Zona::drawPrc(Color c, Color txt_c)
{
    int valor;
    int ancho;
    float width = (float)GetScreenWidth();
    valor = ((timer[getIntFromColor(c)] / 60.0f) / 10.0f) * 100;
    string txt = to_string(valor) + "%";
    ancho = MeasureText(txt.c_str(), 60);
    Vector2 pos = {width / 2 - ancho / 2 + 12, 35};
    DrawText(txt.c_str(), pos.x, pos.y, 60, txt_c);
}
