#pragma once 
#include "defines.h"


// - - - Forward declaration
struct game;


// - - - Application Configuration - - -

typedef struct applicationConfig
{
    short startPositionX;
    short startPositionY;
    short startWidth;
    short startHeight;
    char* name;
} applicationConfig;


// - - - | Application Functions | - - -


FORGE_API bool createApplication(struct game* GAME);

FORGE_API bool runApplication();

void applicationGetFrameBufferSize(unsigned int* WIDTH, unsigned int* HEIGHT);
