#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/memory.h"


// - - - | Renderer Frontend | - - -


// - - - Renderer Frontend Struct (Class)
typedef struct rendererSystemState
{
    rendererBackend backend;
} rendererSystemState;

static rendererSystemState* statePtr;


// - - - Renderer Frontend Class Methods - - -

void rendererSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APPLICATION)
{
    *MEMORY_REQUIREMENT = sizeof(rendererSystemState);
    if (STATE == 0)
    {
        return;
    }

    statePtr = STATE;

    //TODO: make this configurable
    rendererBackendCreate(RENDERER_VULKAN, &statePtr->backend);
    statePtr->backend.frameNumber = 0;

    if (!statePtr->backend.initialize(&statePtr->backend, APPLICATION))
    {
        FORGE_LOG_FATAL("Renderer Backend Failed to Create!");
        return;
    }

    FORGE_LOG_INFO("Renderer Backend Initialized");
    return;
}

void rendererSystemShutdown(void* STATE)
{
    if (statePtr)
    {
        statePtr->backend.shutdown(&statePtr->backend);
    }
    statePtr = 0;
    FORGE_LOG_INFO("Renderer Backend Shutdown");
}

bool rendererBeginFrame(float DELTA_TIME)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Renderer System not initialized");
        return false;
    }
    return statePtr->backend.beginFrame(&statePtr->backend, DELTA_TIME);
}

bool rendererEndFrame(float DELTA_TIME)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Renderer System not initialized");
        return false;
    }
    bool result = statePtr->backend.endFrame(&statePtr->backend, DELTA_TIME);
    ++statePtr->backend.frameNumber;
    return result;
}

void rendererResized(unsigned short WIDTH, unsigned short HEIGHT)
{
    if (statePtr)
    {
        statePtr->backend.resized(&statePtr->backend, WIDTH, HEIGHT);
    }
    else 
    {
        FORGE_LOG_WARNING("Renderer Backend not initialized to accept resize event: %i %i", WIDTH, HEIGHT);
    }
}

bool rendererDrawFrame(rendererPacket *PACKET)
{
    if (rendererBeginFrame(PACKET->deltaTime))
    {
        if (!rendererEndFrame(PACKET->deltaTime))
        {
            FORGE_LOG_ERROR("Failed to finish frame: %llu", statePtr->backend.frameNumber);
            return false;
        }
    }

    return true;
}
