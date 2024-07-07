#pragma once
#include "vulkan_types.h"

void vulkanFramebufferCreate(
    vulkanContext* CONTEXT,
    vulkanRenderpass* RENDERPASS,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    unsigned int ATTACHMENT_COUNT,
    VkImageView* ATTACHMENTS,
    vulkanFramebuffer* OUTPUT_FRAMEBUFFER);

void vulkanFramebufferDestroy(vulkanContext* CONTEXT, vulkanFramebuffer* FRAMEBUFFER);
