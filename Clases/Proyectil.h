#pragma once
#include "..\t.h"
#include "Obstaculo.h"

class Proyectil
{
private:
    int rebotes_cont;
    Vector2 centro;
    Vector2 vel;
    Color color;
    float radio;

    Vector2 right(){return {centro.x+radio,centro.y};};
    Vector2 left(){return {centro.x-radio,centro.y};};
    Vector2 down(){return {centro.x,centro.y+radio};};
    Vector2 up(){return {centro.x,centro.y-radio};};
    void colisionObs(list<Obstaculo> obstaculos);
    void colisionBorde();
    void movimiento();

public:
    bool should_del;

    Proyectil(Vector2 fuente,float direccion,Color color);
    bool operator==(const Proyectil& p) const;
    void update(list<Obstaculo> obstaculos);
    Vector2 getCentro(){return centro;};
    Color getColor(){return color;};
    Proyectil(){};
    void draw();
};