#pragma once
#include "C:\raylib\w64devkit\x86_64-w64-mingw32\include\raylib.h"
#include <iostream>
#include <cmath>
#include <list>
#include <string.h>
#include <map>

using std::map;
using std::cout;
using std::list;

#define RANCHO 1080.f
#define RALTO 720.f

#define CASILLA 40.f
#define BORDE_LEFT float(CASILLA*3)
#define BORDE_UP float(CASILLA)
#define BORDE_RIGHT float(RANCHO - CASILLA*3)
#define BORDE_DOWN float(RALTO - CASILLA*2)


#define TANKV 3.f

#define TANKH 60.f
#define TANKW 80.f

#define TANKDIAG sqrt((TANKH/2.f)*(TANKH/2.f) + (TANKW/2.f)*(TANKW/2.f))

#define VEL_PYTL 4.f
#define RAD_PYTL 8.f
#define DELTA_DIR 2.2
#define VEL_ATK 60

#define SPAWN1 {RANCHO / 2.0, RALTO / 2.0}
#define SPAWN2 {BORDE_UP + 300.f, BORDE_LEFT + 300.f}