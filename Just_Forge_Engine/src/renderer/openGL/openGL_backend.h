#pragma once
#include "renderer/renderer_backend.h"

bool openGLRendererBackendInitialize(rendererBackend* BACKEND, const char* APPLICATION, struct platformState* PLATFORM);

void openGLRendererBackendShutdown(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

bool openGLRendrerBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);

bool openGLRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);