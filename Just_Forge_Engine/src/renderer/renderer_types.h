#pragma once
#include "defines.h"
#include "math/math_types.h"

// - - - Global UBO
typedef struct globalUBO
{
    Matrix4 projection; //64 bytes
    Matrix4 view; // 64 bytes
    Matrix4 padding[2]; // 128 bytes
} globalUBO; //256 bytes = 0.25 kB


// - - - | Renderer Backends | - - -

// - - - Types
typedef enum rendererBackendType 
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} rendererBackendType;

// - - - Backend
typedef struct rendererBackend 
{
    unsigned long long frame_number;

    bool (*initialize)(struct rendererBackend* BACKEND, const char* APP_NAME);

    void (*shutdown)(struct rendererBackend* BACKEND);

    void (*resized)(struct rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

    bool (*beginFrame)(struct rendererBackend* BACKEND, float DELTA_TIME);
    void (*updateGlobalState)(Matrix4 PROJECTION, Matrix4 VIEW, Vector3D VIEW_POS, Vector4D AMBIENT_COLOR, int MODE);
    bool (*endFrame)(struct rendererBackend* BACKEND, float DELTA_TIME);    
} rendererBackend;

// - - - Packets
typedef struct renderPacket 
{
    float deltaTime;
} renderPacket;
