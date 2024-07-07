#include "game.h"
#include <entry.h>
#include <core/memory.h>


//Define the function to create a game
bool createGame(game* OUTPUT_GAME)
{
    OUTPUT_GAME->config.startPositionX = 0;
    OUTPUT_GAME->config.startPositionY = 0;
    OUTPUT_GAME->config.startWidth = 1500;
    OUTPUT_GAME->config.startHeight = 720;
    OUTPUT_GAME->config.name = "Just Forge Tester";
    OUTPUT_GAME->init = gameInit;
    OUTPUT_GAME->update = gameUpdate;
    OUTPUT_GAME->render = gameRender;
    OUTPUT_GAME->onResize = gameOnResize;

    OUTPUT_GAME->state = forgeAllocateMemory(sizeof(gameState), MEMORY_TAG_GAME);
    OUTPUT_GAME->applicationState = 0;

    return true;
}


