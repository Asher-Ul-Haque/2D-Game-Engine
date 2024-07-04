#include "game.h"
#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>
#include <core/memory.h>

bool gameInit(game* GAME)
{
    FORGE_LOG_INFO("Game initialised");
    return true;
}

bool gameUpdate(game* GAME, float DELTA_TIME)
{
    static unsigned long long allocCount = 0;
    unsigned long long prevAllocCount = allocCount;
    allocCount = forgeGetMemoryAllocCount();
    if (inputIsKeyUp(KEY_F1) && inputWasKeyDown(KEY_F1))
    {
        FORGE_LOG_INFO("Memory allocations: %llu", forgeGetMemoryAllocCount() - allocCount);
        allocCount = forgeGetMemoryAllocCount();
    }
    if (inputIsKeyUp(KEY_F2) && inputWasKeyDown(KEY_F2))
    {
        FORGE_LOG_INFO(forgeGetMemoryStats());
    }
    return true;
}

bool gameRender(game* GAME, float DELTA_TIME)
{
//    FORGE_LOG_TRACE("Game rendered");
    return true;
}

void gameOnResize(game* GAME, unsigned int WIDTH, unsigned int HEIGHT)
{
    FORGE_LOG_DEBUG("Game resized");
}
