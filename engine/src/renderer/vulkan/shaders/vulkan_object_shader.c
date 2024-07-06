#include "vulkan_object_shader.h"

#include "core/logger.h"
#include "core/memory.h"
#include "math/math_types.h"

#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

bool createObjectShader(vulkanContext* CONTEXT, vulkanObjectShader* SHADER)
{
    char stageTypeStrings[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        if (!createShaderModule(CONTEXT, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrings[i], stageTypes[i], i, SHADER->stages)) 
        {
            FORGE_LOG_ERROR("Unable to create %s shader module for '%s'", stageTypeStrings[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    //TODO: Descriptors

    // Pipleline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float) CONTEXT->framebufferHeight;
    viewport.width = (float) CONTEXT->framebufferWidth;
    viewport.height = -(float) CONTEXT->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = CONTEXT->framebufferWidth;
    scissor.extent.height = CONTEXT->framebufferHeight;

    //Attributes
    unsigned int offset = 0;
    const int attributeCount = 1;
    VkVertexInputAttributeDescription attributes[attributeCount];

    // Position
    VkFormat formats[attributeCount] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    unsigned long long sizes[attributeCount] = {
        sizeof(Vector3D)
    };
    for (unsigned int i = 0; i < attributeCount; ++i)
    {
        attributes[i].binding = 0; //binding index- should match binding desc
        attributes[i].location = i; // attrib location
        attributes[i].format = formats[i];
        attributes[i].offset = offset;
        offset += sizes[i]; 
    }

    //Stages
    //NOTE: should match the number of shader->stages
    VkPipelineShaderStageCreateInfo stageCreateInfo[OBJECT_SHADER_STAGE_COUNT];
    forgeZeroMemory(stageCreateInfo, sizeof(stageCreateInfo));
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        stageCreateInfo[i].sType = SHADER->stages[i].stageCreateInfo.sType;
        stageCreateInfo[i] = SHADER->stages[i].stageCreateInfo;
    }

    if (!createGraphicsPipeline(CONTEXT, 
                                &CONTEXT->mainRenderpass, 
                                attributeCount, 
                                attributes, 
                                0, 
                                0, 
                                OBJECT_SHADER_STAGE_COUNT, 
                                stageCreateInfo, 
                                viewport, 
                                scissor, 
                                false, 
                                &SHADER->pipeline))
    {
        FORGE_LOG_ERROR("Failed to create graphics pipeline for object shader");
        return false;
    }
    FORGE_LOG_DEBUG("Graphics pipeline created");

    return true;
}

void destroyObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER)
{
    destroyGraphicsPipeline(CONTEXT, &SHADER->pipeline);

    //destroy shader modules
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i)
    {
        vkDestroyShaderModule(CONTEXT->device.logicalDevice, SHADER->stages[i].handle, CONTEXT->allocator);
        SHADER->stages[i].handle = 0;
    }
}

void useObjectShader(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER)
{
    unsigned int imageIndex = CONTEXT->imageIndex;
    bindGraphicsPipeline(&CONTEXT->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &SHADER->pipeline);
}

