#pragma once
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/renderer_types.h"

bool vulkanObjectShaderCreate(vulkanContext* CONTEXT, vulkanObjectShader* OUTPUT_SHADER);

void vulkanObjectShaderDestroy(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

void vulkanObjectShaderUse(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

void vulkanObjectShaderUpdateGlobalState(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

