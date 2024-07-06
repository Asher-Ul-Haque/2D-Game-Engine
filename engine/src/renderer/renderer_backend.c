#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"

// - - - Renderer Backend Functions - - -

bool rendererBackendCreate(rendererBackendType TYPE, rendererBackend* BACKEND)
{
    switch (TYPE)
    {
        case RENDERER_OPENGL:
            //TODO: make an openGL renderer
            return true;

        case RENDERER_NULL:
            return true; // If You dont want a renderer, you could just say so

        case RENDERER_DIRECTX:
            //TODO: make a DirectX renderer
            return true;
        
        case RENDERER_METAL:
            //TODO: make a Metal renderer
            return true;

        case RENDERER_VULKAN:
            BACKEND->initialize = vulkanRendererBackendInitialize;
            BACKEND->shutdown = vulkanRendererBackendShutdown;
            BACKEND->resized = vulkanRendererBackendResized;
            BACKEND->beginFrame = vulkanRendererBackendBeginFrame;
            BACKEND->endFrame = vulkanRendererBackendEndFrame;
            return true;
    }
    return false;
}

void rendererBackendDestroy(rendererBackend* BACKEND)
{
    BACKEND->initialize = 0;
    BACKEND->shutdown = 0;
    BACKEND->resized = 0;
    BACKEND->beginFrame = 0;
    BACKEND->endFrame = 0;
    BACKEND->shutdown(BACKEND);
}
