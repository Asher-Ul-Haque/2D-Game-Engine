#include "vulkan_object_shader.h"

#include "core/logger.h"
#include "core/memory.h"
#include "math/math_types.h"

#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


bool vulkanObjectShaderCreate(vulkanContext* CONTEXT, vulkanObjectShader* OUTPUT_SHADER) 
{
    // Shader module init per stage.
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        if (!createShaderModule(CONTEXT, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i], stageTypes[i], i, OUTPUT_SHADER->stages)) 
        {
            FORGE_LOG_ERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // TODO: Descriptors

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)CONTEXT->framebufferHeight;
    viewport.width = (float)CONTEXT->framebufferWidth;
    viewport.height = -(float)CONTEXT->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = CONTEXT->framebufferWidth;
    scissor.extent.height = CONTEXT->framebufferHeight;

    // Attributes
    unsigned int offset = 0;
    const int attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
    // Position
    VkFormat formats[attribute_count] = 
    {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    unsigned long long sizes[attribute_count] = 
    {
        sizeof(Vector3D)
    };
    for (unsigned int i = 0; i < attribute_count; ++i) 
    {
        attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
        attribute_descriptions[i].location = i;  // attrib location
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    // TODO: Desciptor set layouts.

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stageCreateInfos[OBJECT_SHADER_STAGE_COUNT];
    forgeZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        stageCreateInfos[i].sType = OUTPUT_SHADER->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = OUTPUT_SHADER->stages[i].shaderStageCreateInfo;
    }

    if (!vulkanGraphicsPipelineCreate(
            CONTEXT,
            &CONTEXT->mainRenderpass,
            attribute_count,
            attribute_descriptions,
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfos,
            viewport,
            scissor,
            false,
            &OUTPUT_SHADER->pipeline)) 
    {
        FORGE_LOG_ERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    return true;
}

void vulkanObjectShaderDestroy(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER) 
{
    vulkanPipelineDestroy(CONTEXT, &SHADER->pipeline);

    // Destroy shader modules.
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        vkDestroyShaderModule(CONTEXT->device.logicalDevice, SHADER->stages[i].handle, CONTEXT->allocator);
        SHADER->stages[i].handle = 0;
    }
}

void vulkanObjectShaderUse(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER) 
{
    unsigned int imageIndex = CONTEXT->imageIndex;
    vulkanPipelineBind(&CONTEXT->graphicsCommandBUffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &SHADER->pipeline);
}