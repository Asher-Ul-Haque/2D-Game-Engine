#pragma once
#include "vulkan_types.h"
#include <vulkan/vulkan_core.h>

bool createGraphicsPipeline(
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
    vulkanPipeline* PIPELINE);

void destroyGraphicsPipeline(vulkanContext* CONTEXT, vulkanPipeline* PIPELINE);

void bindGraphicsPipeline(vulkanCommandBuffer* COMMAND_BUFFER, VkPipelineBindPoint BIND_POINT, vulkanPipeline* PIPELINE);
