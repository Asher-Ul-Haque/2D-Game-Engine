#pragma once
#include <defines.h>
#include <game_types.h>


// - - - | Game Structure and Functions | - - -


// - - - Game Structure - - -

typedef struct gameState
{
    float deltaTime;
} gameState;


// - - - Game Functions - - -

bool gameInit(game* GAME);

bool gameUpdate(game* GAME, float DELTA_TIME);

bool gameRender(game* GAME, float DELTA_TIME);

void gameOnResize(game* GAME, unsigned int WIDTH, unsigned int HEIGHT);
