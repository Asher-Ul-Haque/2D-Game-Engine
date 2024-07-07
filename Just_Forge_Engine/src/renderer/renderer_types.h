#pragma once
#include "defines.h"


// - - - | Renderer Backends | - - -

// - - - Types
typedef enum rendererBackendType 
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} rendererBackendType;

// - - - Backend
typedef struct rendererBackend {
    unsigned long long frame_number;

    bool (*initialize)(struct rendererBackend* BACKEND, const char* APP_NAME);

    void (*shutdown)(struct rendererBackend* BACKEND);

    void (*resized)(struct rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

    bool (*beginFrame)(struct rendererBackend* BACKEND, float DELTA_TIME);
    bool (*endFrame)(struct rendererBackend* BACKEND, float DELTA_TIME);    
} rendererBackend;

// - - - Packets
typedef struct renderPacket 
{
    float deltaTime;
} renderPacket;