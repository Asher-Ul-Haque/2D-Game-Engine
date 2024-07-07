#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"


// - - - Backend Functions - - - 

bool createRendererBackend(rendererBackendType TYPE, rendererBackend* OUTPUT_BACKEND) 
{
    if (TYPE == RENDERER_BACKEND_TYPE_VULKAN) 
    {
        OUTPUT_BACKEND->initialize = vulkanRendererBackendInitialize;
        OUTPUT_BACKEND->shutdown = vulkanRendererBackendShutdown;
        OUTPUT_BACKEND->beginFrame = vulkanRendererBackendBeginFrame;
        OUTPUT_BACKEND->endFrame = vulkanRendererBackendEndFrame;
        OUTPUT_BACKEND->resized = vulkanRendererBackendOnResized;

        return true;
    }

    return false;
}

void destroyRendererBackend(rendererBackend* BACKEND) 
{
    BACKEND->initialize = 0;
    BACKEND->shutdown = 0;
    BACKEND->beginFrame = 0;
    BACKEND->endFrame = 0;
    BACKEND->resized = 0;
}