#pragma once
#include "vulkan_types.h"


// - - - Renderpass Functions - - -

void vulkanRenderpassCreate(
    vulkanContext* CONTEXT, 
    vulkanRenderpass* OUTPUT_RENDERPASS,
    float X, float Y, float W, float H,
    float R, float G, float B, float A,
    float DEPTH,
    unsigned int STENCIL);

void vulkanRenderpassDestroy(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS);

void vulkanRenderpassBegin(
    vulkanCommandBuffer* COMMAND_BUFFER, 
    vulkanRenderpass* RENDERPASS,
    VkFramebuffer FRAME_BUFFER);

void vulkanRenderpassEnd(vulkanCommandBuffer* COMMAND_BUFFER, vulkanRenderpass* RENDERPASS);