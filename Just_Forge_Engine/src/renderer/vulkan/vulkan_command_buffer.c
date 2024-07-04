#include "core/memory.h"
#include "vulkan_types.h"
#include <vulkan/vulkan_core.h>


// - - - | Command buffer functions | - - -


// - - - Allocation - - -

void commandBufferAllocate(vulkanContext* CONTEXT, VkCommandPool COMMAND_POOL, bool IS_PRIMARY, vulkanCommandBuffer* COMMAND_BUFFER)
{
    forgeZeroMemory(COMMAND_BUFFER, sizeof(COMMAND_BUFFER));
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = COMMAND_POOL;
    allocateInfo.level = IS_PRIMARY ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = 0;

    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_NONE;
    VK_CHECK(vkAllocateCommandBuffers(CONTEXT->device.logicalDevice, &allocateInfo, &COMMAND_BUFFER->handle));
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_READY;
}

void commandBufferFree(vulkanContext* CONTEXT, VkCommandPool COMMAND_POOL, vulkanCommandBuffer* COMMAND_BUFFER)
{
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_NONE;
    COMMAND_BUFFER->handle = 0;
}


// - - - Recording - - -

void commandBufferBegin(vulkanCommandBuffer* COMMAND_BUFFER, bool IS_SINGLE_USE, bool IS_SIMULTANEOUS_USE, bool IS_RENDERPASS_CONTINUE)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if (IS_SINGLE_USE)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (IS_SIMULTANEOUS_USE)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }
    if (IS_RENDERPASS_CONTINUE)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(COMMAND_BUFFER->handle, &beginInfo));
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_RECORDING;
}

void commandBufferEnd(vulkanCommandBuffer* COMMAND_BUFFER)
{
    VK_CHECK(vkEndCommandBuffer(COMMAND_BUFFER->handle));
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_FINISHED;
}

void commandBufferUpdateSubmits(vulkanCommandBuffer* COMMAND_BUFFER)
{
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void commandBufferReset(vulkanCommandBuffer* COMMAND_BUFFER)
{
    COMMAND_BUFFER->state = COMMAND_BUFFER_STATE_READY;
}

// - - - Single use - - -

void commandBufferBeginSingleUse(vulkanContext* CONTEXT, VkCommandPool POOL, vulkanCommandBuffer* COMMAND_BUFFER)
{
    commandBufferAllocate(CONTEXT, POOL, true, COMMAND_BUFFER);
    commandBufferBegin(COMMAND_BUFFER, true, false, false);
}

void commandBufferEndSingleUse(vulkanContext* CONTEXT, VkCommandPool POOL, vulkanCommandBuffer* COMMAND_BUFFER, VkQueue QUEUE)
{
    commandBufferEnd(COMMAND_BUFFER);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &COMMAND_BUFFER->handle;
    VK_CHECK(vkQueueSubmit(QUEUE, 1, &submitInfo, 0));

    // Wait for the queue to finish
    VK_CHECK(vkQueueWaitIdle(QUEUE));

    commandBufferFree(CONTEXT, POOL, COMMAND_BUFFER);
}
