#include <stdio.h>                 
#include <vector>        
#include <SDL2/SDL.h>    
#include "NaiveRasterizer.h"
#include "Controller.h"
#include "ControllerState.h"
#include <ctime>
#include <chrono>

#define HEIGHT 512
#define WIDTH 512

const double TICK_RATE = 1. / 128.;
const double MAX_DT = 0.25;  

#ifndef ENGINE_H
#define ENGINE_H

class Engine {
public:
    Engine();
    int MainLoop();
private:
};


#endif