#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/memory.h"

#include "math/forge_math.h"
#include "math/math_types.h"


// - - - Rendering System - - - -


typedef struct rendererSystemState 
{
    rendererBackend backend;
} rendererSystemState;

static rendererSystemState* statePtr;


// - - - Rendering Functions - - - 

bool rendererSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APP_NAME) 
{
    *MEMORY_REQUIREMENT = sizeof(rendererSystemState);
    if (STATE == 0) 
    {
        return true;
    }
    statePtr = STATE;

    // TODO: make this configurable.
    createRendererBackend(RENDERER_BACKEND_TYPE_VULKAN, &statePtr->backend);
    statePtr->backend.frame_number = 0;

    if (!statePtr->backend.initialize(&statePtr->backend, APP_NAME)) 
    {
        FORGE_LOG_FATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

void rendererSystemShutdown(void* STATE) 
{
    if (statePtr) 
    {
        statePtr->backend.shutdown(&statePtr->backend);
    }
    statePtr = 0;
}

bool rendererBeginFrame(float DELTA_TIME) 
{
    if (!statePtr) 
    {
        return false;
    }
    return statePtr->backend.beginFrame(&statePtr->backend, DELTA_TIME);
}

bool rendererEndFrame(float DELTA_TIME) 
{
    if (!statePtr) 
    {
        return false;
    }
    bool result = statePtr->backend.endFrame(&statePtr->backend, DELTA_TIME);
    statePtr->backend.frame_number++;
    return result;
}

void rendererSystemResize(unsigned short WIDTH, unsigned short HEIGHT) 
{
    if (statePtr) 
    {
        statePtr->backend.resized(&statePtr->backend, WIDTH, HEIGHT);
    } 
    else 
    {
        FORGE_LOG_WARNING("renderer backend does not exist to accept resize: %i %i", WIDTH, HEIGHT);
    }
}

bool rendererSystemDrawFrame(renderPacket* PACKET) 
{
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (rendererBeginFrame(PACKET->deltaTime)) 
    {
        static float z = -1.0f;
        static float x = 0.0f;
        static float y = 0.0f;
        Matrix4 projection = perspectiveMatrix(degreeToRadian(45.0f), 1280/720.0f, 0.1f, 1000.0f);
        Matrix4 view = translationMatrix4((Vector3D){x, y, z});
        statePtr->backend.updateGlobalState(projection, view, zeroVector3D(), oneVector4D(), 0); // TODO: temp
        z -= 0.01f;
        x += 0.01f;
        y += 0.01f;
        // End the frame. If this fails, it is likely unrecoverable.
        bool result = rendererEndFrame(PACKET->deltaTime);

        if (!result) 
        {
            FORGE_LOG_ERROR("rendererEndFrame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}
