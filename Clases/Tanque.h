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

class Tanque
{
private:
    Rectangle pos;
    float direccion;
    Vector2 vel;

    int escudo_duracion;
    Vector2 v_u_front;
    bool colision_obs;
    Vector2 v_u_side;
    Vector2 last_pos;
    float area_total;
    int escudo_timer;
    Triangle front;
    Triangle right;
    int disp_timer;
    Triangle left;
    Vector2 spawn;
    Triangle back;
    int frame_izq;
    int frame_der;
    bool escudo_b;
    float dir_vel;
    bool in_obs;
    int right_c;
    Color color;
    int left_c;
    int acel_c;
    int vida;
    int ret_c;
    
    void area();
    void calcularDireccion();
    void calcularVelocidad();
    void movimiento();
    void colisionBorde();
    // Vector2 calcularVertices(function<float(float ,float)> f);
    float areaTriangulo(Vector2 vertice1,Vector2 vertice2,Vector2 vertice3);
    void vectorUnitario();
    void animacion();
    void colisionObs(list<Obstaculo> obstaculos);
    void setSpawn();
    void respawn();

public:
    bool click_b;
    bool clickr_b;
    bool acel_b;
    bool ret_b;
    bool left_b;
    bool right_b;
    bool should_del;

    Tanque(Color color);
    Tanque(){};
    Proyectil disparar();
    Color getColor(){return color;};
    void input();
    void update(list<Obstaculo> obstaculos);
    void draw(array<Texture2D, 3> asset_tanque);
    bool operator==(const Tanque &t) const;
    bool colisionProyectiles(list<Proyectil> &proyectiles);
    Vector2 getCentro(){return Vector2{pos.x,pos.y};};
    Rectangle getRec(){return pos;};
    Vector2 canon();
    int getEscudoTimer(){return escudo_timer;};
    int getVida(){return vida;};
    void danio(){vida -= TANK_DANIO;};
    void drawVida();
    bool escudo();
    bool canDisp();

};