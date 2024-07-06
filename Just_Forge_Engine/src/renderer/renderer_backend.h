#pragma once
#include "renderer_types.h"

struct platformState;


// - - - Backend Functions - - -

bool createRendererBackend(rendererBackendType TYPE, rendererBackend* OUTPUT_BACKEND);
void destroyRendererBackend(rendererBackend* BACKEND);