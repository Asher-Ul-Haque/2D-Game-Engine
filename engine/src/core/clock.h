#pragma once
#include "defines.h"


// - - - Clock - - -

typedef struct clock 
{
    double startTime;
    double elapsedTime;
} clock;


// - - - Clock Functions - - -

FORGE_API void clockUpdate(clock* CLOCK);
FORGE_API void clockStart(clock* CLOCK);
FORGE_API void clockStop(clock* CLOCK);
