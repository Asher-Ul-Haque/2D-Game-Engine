#include "vulkan_command_buffer.h"
#include "core/memory.h"

void vulkanCommandBufferAllocate(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    bool IS_PRIMARY,
    vulkanCommandBuffer* OUTPUT_COMMAND_BUFFER) 
{
    forgeZeroMemory(OUTPUT_COMMAND_BUFFER, sizeof(OUTPUT_COMMAND_BUFFER));

    VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate_info.commandPool = POOL;
    allocate_info.level = IS_PRIMARY ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = 0;

    OUTPUT_COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        CONTEXT->device.logicalDevice,
        &allocate_info,
        &OUTPUT_COMMAND_BUFFER->handle));
    OUTPUT_COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferFree(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* COMMAND_BUFFER) 
{
    vkFreeCommandBuffers(
        CONTEXT->device.logicalDevice,
        POOL,
        1,
        &COMMAND_BUFFER->handle);

    COMMAND_BUFFER->handle = 0;
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkanCommandBufferBegin(
    vulkanCommandBuffer* COMMAND_BUFFER,
    bool IS_SINGLE_USE,
    bool IS_RENDERPASS_CONTINUE,
    bool IS_SIMULTANEOUS_USE) 
{    
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
    if (IS_SINGLE_USE) 
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (IS_RENDERPASS_CONTINUE) 
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (IS_SIMULTANEOUS_USE) 
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(COMMAND_BUFFER->handle, &beginInfo));
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkanCommandBufferEnd(vulkanCommandBuffer* COMMAND_BUFFER) 
{
    VK_CHECK(vkEndCommandBuffer(COMMAND_BUFFER->handle));
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer* COMMAND_BUFFER) 
{
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkanCommandBufferReset(vulkanCommandBuffer* COMMAND_BUFFER) 
{
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* OUTPUT_COMMAND_BUFFER) 
{
    vulkanCommandBufferAllocate(CONTEXT, POOL, true, OUTPUT_COMMAND_BUFFER);
    vulkanCommandBufferBegin(OUTPUT_COMMAND_BUFFER, true, false, false);
}

void vulkanCommandBufferEndSingleUse(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* COMMAND_BUFFER,
    VkQueue QUEUE) 
{
    // End the command buffer.
    vulkanCommandBufferEnd(COMMAND_BUFFER);

    // Submit the queue
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &COMMAND_BUFFER->handle;
    VK_CHECK(vkQueueSubmit(QUEUE, 1, &submit_info, 0));

    // Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(QUEUE));

    // Free the command buffer.
    vulkanCommandBufferFree(CONTEXT, POOL, COMMAND_BUFFER);
 }