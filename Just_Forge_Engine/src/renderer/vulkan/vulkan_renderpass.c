#include "vulkan_renderpass.h"
#include "core/memory.h"

void vulkanRenderpassCreate(
    vulkanContext* CONTEXT, 
    vulkanRenderpass* OUTPUT_RENDERPASS,
    float X, float Y, float W, float H,
    float R, float G, float B, float A,
    float DEPTH,
    unsigned int STENCIL) 
{
    OUTPUT_RENDERPASS->x = X;
    OUTPUT_RENDERPASS->y = Y;
    OUTPUT_RENDERPASS->w = W;
    OUTPUT_RENDERPASS->h = H;

    OUTPUT_RENDERPASS->r = R;
    OUTPUT_RENDERPASS->g = G;
    OUTPUT_RENDERPASS->b = B;
    OUTPUT_RENDERPASS->a = A;

    OUTPUT_RENDERPASS->depth = DEPTH;
    OUTPUT_RENDERPASS->stencil = STENCIL;

    // Main subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Attachments TODO: make this configurable.
    const unsigned int attachmentDescriptionCount = 2;
    VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];

    // Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = CONTEXT->swapchain.imageFormat.format;  // TODO: configurable
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Do not expect any particular layout before render pass starts.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Transitioned to after the render pass
    colorAttachment.flags = 0;

    attachmentDescriptions[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;  // Attachment description array index
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    // Depth attachment, if there is one
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = CONTEXT->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1] = depthAttachment;

    // Depth attachment reference
    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // TODO: other attachment types (input, resolve, preserve)

    // Depth stencil data.
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    // Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    // Attachments used for multisampling colour attachments
    subpass.pResolveAttachments = 0;

    // Attachments not used in this subpass, but must be preserved for the next.
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    // Render pass dependencies. TODO: make this configurable.
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    // Render pass create.
    VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentDescriptionCount;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = 0;
    renderPassCreateInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        CONTEXT->device.logicalDevice,
        &renderPassCreateInfo,
        CONTEXT->allocator,
        &OUTPUT_RENDERPASS->handle));
}

void vulkanRenderpassDestroy(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS)
{
    if (RENDERPASS && RENDERPASS->handle) 
    {
        vkDestroyRenderPass(CONTEXT->device.logicalDevice, RENDERPASS->handle, CONTEXT->allocator);
        RENDERPASS->handle = 0;
    }
}

void vulkanRenderpassBegin(
    vulkanCommandBuffer* COMMAND_BUFFER,
    vulkanRenderpass* RENDERPASS,
    VkFramebuffer FRAME_BUFFER) 
{
    VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = RENDERPASS->handle;
    beginInfo.framebuffer = FRAME_BUFFER;
    beginInfo.renderArea.offset.x = RENDERPASS->x;
    beginInfo.renderArea.offset.y = RENDERPASS->y;
    beginInfo.renderArea.extent.width = RENDERPASS->w;
    beginInfo.renderArea.extent.height = RENDERPASS->h;

    VkClearValue clearValues[2];
    forgeZeroMemory(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = RENDERPASS->r;
    clearValues[0].color.float32[1] = RENDERPASS->g;
    clearValues[0].color.float32[2] = RENDERPASS->b;
    clearValues[0].color.float32[3] = RENDERPASS->a;
    clearValues[1].depthStencil.depth = RENDERPASS->depth;
    clearValues[1].depthStencil.stencil = RENDERPASS->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(COMMAND_BUFFER->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkanRenderpassEnd(vulkanCommandBuffer* COMMAND_BUFFER, vulkanRenderpass* RENDERPASS) 
{
    vkCmdEndRenderPass(COMMAND_BUFFER->handle);
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_RECORDING;
}