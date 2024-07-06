#pragma once
#include "core/application.h"


// - - - Game Structure - - -

typedef struct game 
{
    applicationConfig config;
    bool (* init)(struct game* GAME);
    bool (* update)(struct game* GAME, float DELTA_TIME);
    bool (* render)(struct game* GAME, float DELTA_TIME);
    void (* onResize)(struct game* GAME, unsigned int WIDTH, unsigned int HEIGHT);
    void* state; //created and managed by the game
    void* applicationState;
} game;
