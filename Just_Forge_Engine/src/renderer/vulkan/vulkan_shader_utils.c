#include "vulkan_shader_utils.h"
#include "core/logger.h"
#include "core/memory.h"
#include "platform/filesystem.h"
#include <stdio.h>

bool createShaderModule(vulkanContext* CONTEXT, const char* NAME, const char* TYPE_STRING, VkShaderStageFlagBits SHADER_STAGE_FLAG, unsigned int STAGE_INDEX, vulkanShaderStage* SHADER_STAGES)
{
    char fileName[512];
    sprintf(fileName, "Assets/shaders/%s.%s.spv", NAME, TYPE_STRING);

    forgeZeroMemory(&SHADER_STAGES[STAGE_INDEX].createInfo, sizeof(VkShaderModuleCreateInfo));
    SHADER_STAGES[STAGE_INDEX].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    //Obtian the file handle
    File file;
    if (!openFile(fileName, FILE_MODE_READ, true, &file))
    {
        FORGE_LOG_ERROR("Failed to open shader file: %s", fileName);
        return false;
    }

    // Read the entire file as binary
    unsigned long long size = 0;
    unsigned char* buffer = 0;
    if (!readAllBytes(&file, &buffer, &size))
    {
        FORGE_LOG_ERROR("Failed to read shader file: %s", fileName);
        return false;
    }

    SHADER_STAGES[STAGE_INDEX].createInfo.codeSize = size;
    SHADER_STAGES[STAGE_INDEX].createInfo.pCode = (unsigned int*)buffer;

    // Close the file
    closeFile(&file);

    VK_CHECK(vkCreateShaderModule(CONTEXT->device.logicalDevice, 
                                  &SHADER_STAGES[STAGE_INDEX].createInfo, 
                                  CONTEXT->allocator, 
                                  &SHADER_STAGES[STAGE_INDEX].handle));

    // Shader stage info
    forgeZeroMemory(&SHADER_STAGES[STAGE_INDEX].stageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    SHADER_STAGES[STAGE_INDEX].stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    SHADER_STAGES[STAGE_INDEX].stageCreateInfo.stage = SHADER_STAGE_FLAG;
    SHADER_STAGES[STAGE_INDEX].stageCreateInfo.module = SHADER_STAGES[STAGE_INDEX].handle;
    SHADER_STAGES[STAGE_INDEX].stageCreateInfo.pName = "main";

    if (buffer)
    {
        forgeFreeMemory(buffer, sizeof(unsigned char) * size, MEMORY_TAG_FILE);
        buffer = 0;
    }

    return true;
}
