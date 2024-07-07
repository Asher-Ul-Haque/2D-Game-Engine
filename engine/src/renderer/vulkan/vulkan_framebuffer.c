#include "vulkan_framebuffer.h"
#include "core/memory.h"

void vulkanFramebufferCreate(
    vulkanContext* CONTEXT,
    vulkanRenderpass* RENDERPASS,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    unsigned int ATTACHMENT_COUNT,
    VkImageView* ATTACHMENTS,
    vulkanFramebuffer* OUTPUT_FRAMEBUFFER) 
{
    // Take a copy of the attachments, renderpass and attachment count
    OUTPUT_FRAMEBUFFER->attachments = forgeAllocateMemory(sizeof(VkImageView) * ATTACHMENT_COUNT, MEMORY_TAG_RENDERER);
    for (unsigned int i = 0; i < ATTACHMENT_COUNT; ++i) 
    {
        OUTPUT_FRAMEBUFFER->attachments[i] = ATTACHMENTS[i];
    }
    OUTPUT_FRAMEBUFFER->renderpass = RENDERPASS;
    OUTPUT_FRAMEBUFFER->attachment_count = ATTACHMENT_COUNT;

    // Creation info
    VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.renderPass = RENDERPASS->handle;
    framebufferCreateInfo.attachmentCount = ATTACHMENT_COUNT;
    framebufferCreateInfo.pAttachments = OUTPUT_FRAMEBUFFER->attachments;
    framebufferCreateInfo.width = WIDTH;
    framebufferCreateInfo.height = HEIGHT;
    framebufferCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(
        CONTEXT->device.logicalDevice,
        &framebufferCreateInfo,
        CONTEXT->allocator,
        &OUTPUT_FRAMEBUFFER->handle));
}

void vulkanFramebufferDestroy(vulkanContext* CONTEXT, vulkanFramebuffer* FRAMEBUFFER) 
{
    vkDestroyFramebuffer(CONTEXT->device.logicalDevice, FRAMEBUFFER->handle, CONTEXT->allocator);
    if (FRAMEBUFFER->attachments) 
    {
        forgeFreeMemory(FRAMEBUFFER->attachments, sizeof(VkImageView) * FRAMEBUFFER->attachment_count, MEMORY_TAG_RENDERER);
        FRAMEBUFFER->attachments = 0;
    }
    FRAMEBUFFER->handle = 0;
    FRAMEBUFFER->attachment_count = 0;
    FRAMEBUFFER->renderpass = 0;
}