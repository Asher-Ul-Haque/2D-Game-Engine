#pragma once
#include "renderer/renderer_backend.h"

bool metalRendererBackendInitialize(rendererBackend* BACKEND, const char* APPLICATION, struct platformState* PLATFORM);

void metalRendererBackendShutdown(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

bool metalRendrerBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);

bool metalRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);