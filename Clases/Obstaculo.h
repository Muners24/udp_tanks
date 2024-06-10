#pragma once
#include "..\t.h"

class Obstaculo
{
private:
    Rectangle pos;

public:
    Obstaculo(){};
    Obstaculo(Vector2 inicio);
    Rectangle getRec(){return pos;};
    void draw(Texture2D box);
};
