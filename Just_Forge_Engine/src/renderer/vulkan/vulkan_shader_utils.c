#include "vulkan_shader_utils.h"

#include <string.h>
#include <stdio.h>

#include "core/logger.h"
#include "core/memory.h"

#include "platform/filesystem.h"

// - - - Da Function
bool createShaderModule(
    vulkanContext* CONTEXT,
    const char* NAME,
    const char* TYPE_STR,
    VkShaderStageFlagBits SHADER_STAGE_FLAG,
    unsigned int STAGE_INDEX,
    vulkanShaderStage* SHADER_STAGES)     
{
    // Build file name.
    char fileName[512];
    snprintf(fileName, sizeof(fileName), "Assets/shaders/%s.%s.spv", NAME, TYPE_STR);

    forgeZeroMemory(&SHADER_STAGES[STAGE_INDEX].createInfo, sizeof(VkShaderModuleCreateInfo));
    SHADER_STAGES[STAGE_INDEX].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Obtain file handle.
    File handle;
    if (!openFile(fileName, FILE_MODE_READ, true, &handle)) 
    {
        FORGE_LOG_ERROR("Unable to read shader module: %s.", fileName);
        return false;
    }

    // Read the entire file as binary.
    unsigned long long size = 0;
    unsigned char* fileBuffer = 0;
    if (!readAllBytes(&handle, &fileBuffer, &size)) 
    {
        FORGE_LOG_ERROR("Unable to binary read shader module: %s.", fileName);
        return false;
    }
    SHADER_STAGES[STAGE_INDEX].createInfo.codeSize = size;
    SHADER_STAGES[STAGE_INDEX].createInfo.pCode = (unsigned int*)fileBuffer;

    // Close the file.
    closeFile(&handle);

    VK_CHECK(vkCreateShaderModule(
        CONTEXT->device.logicalDevice,
        &SHADER_STAGES[STAGE_INDEX].createInfo,
        CONTEXT->allocator,
        &SHADER_STAGES[STAGE_INDEX].handle));

    // Shader stage info
    forgeZeroMemory(&SHADER_STAGES[STAGE_INDEX].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    SHADER_STAGES[STAGE_INDEX].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    SHADER_STAGES[STAGE_INDEX].shaderStageCreateInfo.stage = SHADER_STAGE_FLAG;
    SHADER_STAGES[STAGE_INDEX].shaderStageCreateInfo.module = SHADER_STAGES[STAGE_INDEX].handle;
    SHADER_STAGES[STAGE_INDEX].shaderStageCreateInfo.pName = "main";


    return true;
}
