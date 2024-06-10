#include "..\t.h"
#include "Tanque.h"

class Zona
{
private:
    Rectangle pos;
    Color color;
    int timer[9];
    bool anime_b;
    void colisionsTanques(list<Tanque*> tanques);
    void transform();
    void animacion();
    void drawPrc(Color c,Color txt_c);
public:
    Zona(){};
    Zona(int x,int y);
    bool colisionTanque(Tanque t);
    void update(list<Tanque*> tanques);
    void getPrc(Tanque t);
    Rectangle getRec(){return pos;};
    Color getColor(){return color;};
    
};