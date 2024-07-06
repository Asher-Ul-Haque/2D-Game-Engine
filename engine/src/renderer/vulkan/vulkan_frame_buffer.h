#include "vulkan_types.h"

void createFrameBuffer(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS, unsigned int WIDTH, unsigned int HEIGHT, unsigned int ATTACHMENT_COUNT, VkImageView* ATTACHMENTS, vulkanFramebuffer* FRAMEBUFFER);

void destroyFrameBuffer(vulkanContext* CONTEXT, vulkanFramebuffer* FRAMEBUFFER);
