#pragma once
#include "../t.h"
#include "Proyectil.h"
#include <functional>

using std::function;
using std::ostream;

typedef struct
{
    Vector2 vertice[3];
}Triangle;

class Tanque
{
private:
    Rectangle pos;
    Color color;
    float direccion;
    Vector2 vel;
    
    Triangle front;
    Triangle left;
    Triangle right;
    Triangle back;
    Vector2 v_u_front;
    Vector2 v_u_side;
    
    Vector2 last_pos;
    int disp_timer;

    void area();
    void calcularDireccion();
    void calcularVelocidad();
    void movimiento();
    Vector2 canon();
    void colisionBorde(bool map[int(RALTO)][int(RANCHO)]);
    //Vector2 calcularVertices(function<float(float ,float)> f);
    
    void vectorUnitario();

public:
    bool click_b;
    bool acel_b;
    bool ret_b;
    bool left_b;
    bool right_b;
    list<Proyectil> proyectiles;
    Tanque(Vector2 spawn, Color color);
    Tanque(){};
    void disparar();
    //void input();
    void update(bool map[int(RALTO)][int(RANCHO)]);
    void draw();
    
};