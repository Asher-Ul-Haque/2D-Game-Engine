#pragma once
#include "vulkan_types.h"


// - - - Da Function
bool createShaderModule(
    vulkanContext* CONTEXT,
    const char* NAME,
    const char* TYPE_STR,
    VkShaderStageFlagBits SHADER_STAGE_FLAG,
    unsigned int STAGE_INDEX,
    vulkanShaderStage* SHADER_STAGES
);