#pragma once
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/renderer_types.h"


bool createObjectShader(vulkanContext* CONTEXT, vulkanObjectShader* SHADER);

void destroyObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);

void useObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER);


