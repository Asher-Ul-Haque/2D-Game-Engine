#pragma once
#include "vulkan_types.h"

bool createShaderModule(vulkanContext* CONTEXT, const char* NAME, const char* TYPE_STRING, VkShaderStageFlagBits SHADER_STAGE_FLAG, unsigned int STAGE_INDEX, vulkanShaderStage* SHADER_STAGES); 
