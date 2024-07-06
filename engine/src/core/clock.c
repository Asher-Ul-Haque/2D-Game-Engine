#include "clock.h"
#include "platform/platform.h"


// - - - Clock Functions - - -

void clockUpdate(clock* CLOCK)
{
    if (CLOCK->startTime != 0)
    {
        CLOCK->elapsedTime = platformGetTime() - CLOCK->startTime;
    }
}

void clockStart(clock* CLOCK)
{
    CLOCK->startTime = platformGetTime();
    CLOCK->elapsedTime = 0;
}

void clockStop(clock* CLOCK)
{
    CLOCK->startTime = 0;
}
