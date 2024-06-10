#pragma once
#include <ws2tcpip.h>
#include <winsock2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <iostream>
#include <sstream>
#include <future>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <array>
#include <cmath>
#include <mutex>
#include <queue>
#include <list>
#include <map>

using std::to_string;
using std::vector;
using std::string;
using std::thread;
using std::queue;
using std::mutex;
using std::array;
using std::list;
using std::cout;
using std::map;

#define RANCHO 1600.f
#define RALTO 960.f

#define CASILLA 80.f
#define BORDE_LEFT 0.0f 
#define BORDE_UP 0.0f
#define BORDE_RIGHT 50*CASILLA
#define BORDE_DOWN 50*CASILLA

#define COLUMNAS BORDE_RIGHT/CASILLA
#define FILAS BORDE_DOWN/CASILLA
#define TANKV 3.3f
#define TANKH 60.f
#define TANKW 80.f
#define TANKDIAG sqrt((TANKH / 2.f) * (TANKH / 2.f) + (TANKW / 2.f) * (TANKW / 2.f))
#define TANK_DANIO 24

#define TANK_ESC_RAD 65
#define TANK_ESC_CD 599
#define TANK_ESC_DUR 90

#define VEL_PYTL 4.8f
#define RAD_PYTL 9.f
#define DELTA_DIR 2.6
#define VEL_ATK 60

#define ZONA_TTT 600
#define ZONA_PRP 50

#define MAX_SOUNDS 20

typedef struct m_s
{
    int id;
    Vector2 origen;
    bool should_rep;
} sound_msg;

#define DISP 0
#define DANIO 1
#define SHIELD 2