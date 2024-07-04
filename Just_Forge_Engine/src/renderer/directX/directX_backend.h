#pragma once
#include "renderer/renderer_backend.h"

bool directXRendererBackendInitialize(rendererBackend* BACKEND, const char* APPLICATION, struct platformState* PLATFORM);

void directXRendererBackendShutdown(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

bool directXRendrerBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);

bool directXRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);