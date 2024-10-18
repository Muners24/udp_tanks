#pragma once
#include "../t.h"
#include "Proyectil.h"
#include <functional>

using std::function;
using std::ostream;

typedef struct
{
    Vector2 vertice[3];

} Triangle;

typedef struct _dir_b
{
    bool right;
    bool left;
    bool up;
    bool down;
} _Dir_b;

typedef struct _dir_c
{
    int right;
    int left;
    int up;
    int down;
} _Dir_c;

typedef struct _tanque
{
    int id;
    int escudo_duracion;
    bool colision_obs;
    Vector2 last_pos;
    int escudo_timer;
    Rectangle pos;
    float direccion;
    Triangle front;
    Triangle right;
    Triangle left;
    Triangle back;
    Vector2 vel;
    int disp_timer;
    Vector2 spawn;
    int frame_izq;
    int frame_der;
    bool escudo_b;
    float dir_vel;
    bool in_obs;
    Color color;
    _Dir_c count;
    int vida;
    bool click_b;
    bool clickr_b;
    _Dir_b wasd;
    bool should_del;
} _Tanque;

class Tanque
{
private:
    
    int escudo_duracion;
    bool colision_obs;
    Vector2 last_pos;
    int escudo_timer;
    Rectangle pos;
    float direccion;
    Triangle front;
    Triangle right;
    Triangle left;
    Triangle back;
    Vector2 vel;
    int disp_timer;
    Vector2 spawn;
    int frame_izq;
    int frame_der;
    bool escudo_b;
    float dir_vel;
    bool in_obs;
    Color color;

    _Dir_c count;
    int vida;

public:
    int id;
    bool click_b;
    bool clickr_b;
    _Dir_b wasd;
    bool should_del;

    Tanque(Color color,int id);
    Tanque(_Tanque &t);
    Tanque() {};
    Proyectil disparar();
    Color getColor() { return color; };
    void input();
    void update(list<Obstaculo> obstaculos);
    void draw(array<Texture2D, 3> asset_tanque);
    bool operator==(const Tanque &t) const;
    bool colisionProyectiles(list<Proyectil> &proyectiles);
    Vector2 getCentro() { return Vector2{pos.x, pos.y}; };
    Rectangle getRec() { return pos; };
    Vector2 canon();
    int getEscudoTimer() { return escudo_timer; };
    int getVida() { return vida; };
    void danio() { vida -= TANK_DANIO; };
    void drawVida();
    bool escudo();
    bool canDisp();
    _Tanque toStruct();

private:
    void area();
    void calcularDireccion();
    void calcularVelocidad();
    void movimiento();
    void colisionBorde();
    // Vector2 calcularVertices(function<float(float ,float)> f);
    float areaTriangulo(Vector2 vertice1, Vector2 vertice2, Vector2 vertice3);
    Vector2 vectorUnitarioFront();
    Vector2 vectorUnitarioSide();
    void animacion();
    void colisionObs(list<Obstaculo> obstaculos);
    void setSpawn();
    void respawn();
    
};

