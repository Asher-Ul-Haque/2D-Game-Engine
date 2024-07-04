#pragma once
#include "renderer_types.h"


// - - - | Renderer Frontend | - - -


// - - - Renderer Frontend Struct (Class) - - -

void rendererSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APPLICATION);

void rendererSystemShutdown(void* STATE);

void rendererResized(unsigned short WIDTH, unsigned short HEIGHT);

bool rendererDrawFrame(rendererPacket* PACKET);
