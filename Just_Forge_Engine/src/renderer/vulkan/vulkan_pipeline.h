#pragma once
#include "vulkan_types.h"

bool vulkanGraphicsPipelineCreate(
    vulkanContext* CONTEXT,
    vulkanRenderpass* RENDERPASS,
    unsigned int ATTRIBUTE_COUNT,
    VkVertexInputAttributeDescription* ATTRIBUTES,
    unsigned int DESCRIPTOR_SET_LAYOUT_COUNT,
    VkDescriptorSetLayout* DESCRIPTOR_SET_LAYOUTS,
    unsigned int STAGE_COUNT,
    VkPipelineShaderStageCreateInfo* STAGES,
    VkViewport VIEWPORT,
    VkRect2D SCISSOR,
    bool IS_WIREFRAME,
    vulkanPipeline* OUTPUT_PIPELINE);

void vulkanPipelineDestroy(vulkanContext* CONTEXT, vulkanPipeline* PIPELINE);

void vulkanPipelineBind(vulkanCommandBuffer* COMMAND_BUFFER, VkPipelineBindPoint BIND_POINT, vulkanPipeline* PIPELINE);
