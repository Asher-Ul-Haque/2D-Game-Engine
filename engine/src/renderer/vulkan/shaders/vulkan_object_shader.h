#pragma once
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/renderer_types.h"

bool vulkanObjectShaderCreate(vulkanContext* CONTEXT, vulkanObjectShader* OUTPUT_SHADER);

<<<<<<< HEAD:engine/src/renderer/vulkan/shaders/vulkan_object_shader.h
bool createObjectShader(vulkanContext* CONTEXT, vulkanObjectShader* SHADER);

void destroyObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

void useObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);
=======
void vulkanObjectShaderDestroy(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/shaders/vulkan_object_shader.h

void vulkanObjectShaderUse(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

