#pragma once
#include "renderer/renderer_backend.h"

bool vulkanRendererBackendInitialize(rendererBackend* BACKEND, const char* APP_NAME);
void vulkanRendererBackendShutdown(rendererBackend* BACKEND);

void vulkanRendererBackendOnResized(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

bool vulkanRendererBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);
bool vulkanRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);