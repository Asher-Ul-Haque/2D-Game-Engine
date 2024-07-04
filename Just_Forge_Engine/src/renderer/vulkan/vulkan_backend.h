#pragma once
#include "renderer/renderer_backend.h"


// - - - | Vulkan as a Renderer Backend | - - -


// - - - Start and Shutdown - - -

bool vulkanRendererBackendInitialize(rendererBackend* BACKEND, const char* APPLICATION);
void vulkanRendererBackendShutdown(rendererBackend* BACKEND);

// - - - Utils - - -
void vulkanRendererBackendResized(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

// - - - Frame - - -
bool vulkanRendererBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);
bool vulkanRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);
