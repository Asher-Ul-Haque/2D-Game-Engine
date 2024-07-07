#pragma once
#include "renderer/renderer_backend.h"

bool vulkanRendererBackendInitialize(rendererBackend* BACKEND, const char* APP_NAME);
void vulkanRendererBackendShutdown(rendererBackend* BACKEND);

void vulkanRendererBackendOnResized(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT);

bool vulkanRendererBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME);

void vulkanRendererBackendUpdateGlobalState(Matrix4 PROJECTION, Matrix4 VIEW, Vector3D VIEW_POS, Vector4D AMBIENCE_COLOR, int MODE);

bool vulkanRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME);
