#include "vulkan_frame_buffer.h"
#include "core/memory.h"
#include <stdlib.h>

void createFrameBuffer(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS, unsigned int WIDTH, unsigned int HEIGHT, unsigned int ATTACHMENT_COUNT, VkImageView* ATTACHMENTS, vulkanFramebuffer* FRAMEBUFFER)
{
    // Make a copy of the attachments, renderpass and attachment count
    FRAMEBUFFER->attachments = forgeAllocateMemory(sizeof(VkImageView) * ATTACHMENT_COUNT, MEMORY_TAG_RENDERER);
    for (unsigned int i = 0; i < ATTACHMENT_COUNT; i++)
    {
        FRAMEBUFFER->attachments[i] = ATTACHMENTS[i];
    }
    FRAMEBUFFER->renderpass = RENDERPASS;
    //Add 0.1 to rgb and 1 to alpha, make sure it is between 0 and 1
    for (int i = 0; i < 3; ++i)
    {
        FRAMEBUFFER->renderpass->clearColor[i] = FRAMEBUFFER->renderpass->clearColor[i] + rand() % 10 / 10.0f;
        if (FRAMEBUFFER->renderpass->clearColor[i] > 1.0f)
        {
            FRAMEBUFFER->renderpass->clearColor[i] = 0.0f;
        }
    }
    FRAMEBUFFER->attachmentCount = ATTACHMENT_COUNT;

    // Create the framebuffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RENDERPASS->handle;
    framebufferInfo.attachmentCount = ATTACHMENT_COUNT;
    framebufferInfo.pAttachments = ATTACHMENTS;
    framebufferInfo.width = WIDTH;
    framebufferInfo.height = HEIGHT;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(CONTEXT->device.logicalDevice, &framebufferInfo, CONTEXT->allocator, &FRAMEBUFFER->handle));
}

void destroyFrameBuffer(vulkanContext* CONTEXT, vulkanFramebuffer* FRAMEBUFFER)
{
    vkDestroyFramebuffer(CONTEXT->device.logicalDevice, FRAMEBUFFER->handle, CONTEXT->allocator);
    if (FRAMEBUFFER->attachments)
    {
        forgeFreeMemory(FRAMEBUFFER->attachments, sizeof(VkImageView) * FRAMEBUFFER->attachmentCount, MEMORY_TAG_RENDERER);
        FRAMEBUFFER->attachments = 0;
    }
    FRAMEBUFFER->attachmentCount = 0;
    FRAMEBUFFER->renderpass = 0;
    FRAMEBUFFER->handle = 0;
}
