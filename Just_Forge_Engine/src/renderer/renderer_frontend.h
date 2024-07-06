#pragma once
#include "renderer_types.h"


// - - - | Renderere Frontend Functions | - - -


bool rendererSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APP_NAME);
void rendererSystemShutdown(void* STATE);

void rendererSystemResize(unsigned short WIDTH, unsigned short HEIGHT);

bool rendererSystemDrawFrame(renderPacket* PACKET);