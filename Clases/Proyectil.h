#pragma once
#include "..\t.h"

class Proyectil
{
private:
    Color color;
    Vector2 vel;
    Vector2 centro;
    float radio;
    int rebotes_cont;
    Vector2 right(){return {centro.x+radio,centro.y};};
    Vector2 up(){return {centro.x,centro.y-radio};};
    Vector2 left(){return {centro.x-radio,centro.y};};
    Vector2 down(){return {centro.x,centro.y+radio};};

    void movimiento();
    void colisionBorde();

public:
    bool should_del;
    void update();
    void draw();
    Proyectil(Vector2 fuente,float direccion,Color color);
};