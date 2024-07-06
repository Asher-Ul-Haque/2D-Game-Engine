#include "vulkan_renderpass.h"
#include <vulkan/vulkan_core.h>
#include "core/logger.h"
#include "core/memory.h"
#include "vulkan_types.h"


// - - - | Rendperass functions | - - -


// - - - Creation - - -

void createRenderpass(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS, float X, float Y, float WIDTH, float HEIGHT, float CLEAR_COLOR[4], float DEPTH, unsigned int PENCIL)
{
    RENDERPASS->x = X;
    RENDERPASS->y = Y;
    RENDERPASS->width = WIDTH;
    RENDERPASS->height = HEIGHT;
    RENDERPASS->clearColor[0] = CLEAR_COLOR[0];
    RENDERPASS->clearColor[1] = CLEAR_COLOR[1];
    RENDERPASS->clearColor[2] = CLEAR_COLOR[2];
    RENDERPASS->clearColor[3] = CLEAR_COLOR[3];
    RENDERPASS->depth = DEPTH;
    RENDERPASS->pencil = PENCIL;
    RENDERPASS->state = RENDERPASS_STATE_READY;

    //Main subpasss
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    //Attachements: TODO: make configurable
    unsigned int attachementCount = 2;
    VkAttachmentDescription attachements[2];

    //Color attachement : TODO: make configurable
    VkAttachmentDescription colorAttachement;
    colorAttachement.format = CONTEXT->swapchain.imageFormat.format;
    colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachement.flags = 0;
    attachements[0] = colorAttachement;
    
    VkAttachmentReference colorAttachementRef = {};
    colorAttachementRef.attachment = 0;
    colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachementRef;

    //Depth attachment, if needed
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = CONTEXT->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachements[1] = depthAttachment;

    VkAttachmentReference depthAttachementRef = {};
    depthAttachementRef.attachment = 1;
    depthAttachementRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass.pDepthStencilAttachment = &depthAttachementRef;
    
    // TODO: other attachements : input, resolve, preserve

    // Depth stencil
    subpass.pDepthStencilAttachment = &depthAttachementRef;

    // Shader input
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    // Attachments used for multisampling color attachments
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    //Renderpass dependencies TODO: make configurable
    VkSubpassDependency dependencies;
    dependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies.dstSubpass = 0;
    dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.srcAccessMask = 0;
    dependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies.dependencyFlags = 0;


    //Render pass create info
    VkRenderPassCreateInfo renderpassInfo = {};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.attachmentCount = attachementCount;
    renderpassInfo.pAttachments = attachements;
    renderpassInfo.subpassCount = 1;
    renderpassInfo.pSubpasses = &subpass;
    renderpassInfo.dependencyCount = 1;
    renderpassInfo.pDependencies = &dependencies;
    renderpassInfo.pNext = 0;
    renderpassInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(CONTEXT->device.logicalDevice, &renderpassInfo, 0, &RENDERPASS->handle));
    FORGE_LOG_INFO("Renderpass created");
}

void destroyRenderpass(vulkanContext* CONTEXT, vulkanRenderpass* RENDERPASS)
{
    if (RENDERPASS && RENDERPASS->handle)
    {
        vkDestroyRenderPass(CONTEXT->device.logicalDevice, RENDERPASS->handle, 0);
        RENDERPASS->handle = 0;
        FORGE_LOG_INFO("Renderpass destroyed");
    }
}


// - - - Usage - - -

void beginRenderpass(vulkanCommandBuffer* COMMAND_BUFFER, vulkanRenderpass* RENDERPASS, VkFramebuffer FRAMEBUFFER)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = RENDERPASS->handle;
    renderPassInfo.framebuffer = FRAMEBUFFER;
    renderPassInfo.renderArea.offset.x = RENDERPASS->x;
    renderPassInfo.renderArea.offset.y = RENDERPASS->y;
    renderPassInfo.renderArea.extent.width = RENDERPASS->width;
    renderPassInfo.renderArea.extent.height = RENDERPASS->height;

    VkClearValue clearValues[2];
    forgeZeroMemory(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = RENDERPASS->clearColor[0];
    clearValues[0].color.float32[1] = RENDERPASS->clearColor[1];
    clearValues[0].color.float32[2] = RENDERPASS->clearColor[2];
    clearValues[0].color.float32[3] = RENDERPASS->clearColor[3];
    clearValues[1].depthStencil.depth = RENDERPASS->depth;
    clearValues[1].depthStencil.stencil = RENDERPASS->pencil;

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(COMMAND_BUFFER->handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_IN_RENDERPASS;
}

void endRenderpass(vulkanCommandBuffer* COMMAND_BUFFER, vulkanRenderpass* RENDERPASS)
{
    vkCmdEndRenderPass(COMMAND_BUFFER->handle);
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_RECORDING;
}
