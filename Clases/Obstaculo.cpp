#include "Obstaculo.h"

Obstaculo::Obstaculo(Vector2 inicio)
{
    int num;
    num = (rand()%3)+1;

    this->pos.x = inicio.x;
    this->pos.y = inicio.y;
    if(num == 1){
        pos.width = CASILLA;
        pos.height = CASILLA;
    } else if(num == 2){
        if(rand()%2){
            pos.width = CASILLA*2;
            pos.height = CASILLA;
        }else{
            pos.width = CASILLA;
            pos.height = CASILLA*2;
        }   
    } else if(num == 3){
        if(rand()%2){
            pos.width = CASILLA*3;
            pos.height = CASILLA*2;
        }else{
            pos.width = CASILLA*2;
            pos.height = CASILLA*3;
        }   
    }   
}

void Obstaculo::draw(Texture2D box)
{   
    DrawRectangleRec(pos,BLACK);
    int x,y;
    for(y = 0;y<pos.height;y+=CASILLA)
    {
         for(x = 0;x<pos.width;x+=CASILLA)
        {
            DrawTexture(box,pos.x+x,pos.y+y,WHITE);
        }
    }
}