#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/memory.h"

bool vulkanBufferCreate(
    vulkanContext* CONTEXT,
    unsigned long long SIZE,
    VkBufferUsageFlagBits USAGE,
    unsigned int MEMORY_PROPERTY_FLAGS,
    bool BIND_ON_CREATE,
    vulkanBuffer* OUTPUT_BUFFER) 
{
    forgeZeroMemory(OUTPUT_BUFFER, sizeof(vulkanBuffer));
    OUTPUT_BUFFER->totalSize = SIZE;
    OUTPUT_BUFFER->usage = USAGE;
    OUTPUT_BUFFER->memoryPropertyFlags = MEMORY_PROPERTY_FLAGS;

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = SIZE;
    bufferInfo.usage = USAGE;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VK_CHECK(vkCreateBuffer(CONTEXT->device.logicalDevice, &bufferInfo, CONTEXT->allocator, &OUTPUT_BUFFER->handle));

    // Gather memory requirements.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(CONTEXT->device.logicalDevice, OUTPUT_BUFFER->handle, &requirements);
    OUTPUT_BUFFER->memoryIndex = CONTEXT->findMemoryIndex(requirements.memoryTypeBits, OUTPUT_BUFFER->memoryPropertyFlags);
    if (OUTPUT_BUFFER->memoryIndex == -1) 
    {
        FORGE_LOG_ERROR("Unable to create vulkan buffer because the required memory type index was not found.");
        return false;
    }

    // Allocate memory info
    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (unsigned int)OUTPUT_BUFFER->memoryIndex;

    // Allocate the memory.
    VkResult result = vkAllocateMemory(
        CONTEXT->device.logicalDevice,
        &allocateInfo,
        CONTEXT->allocator,
        &OUTPUT_BUFFER->memory);

    if (result != VK_SUCCESS) 
    {
        FORGE_LOG_ERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    if (BIND_ON_CREATE) 
    {
        vulkanBufferBind(CONTEXT, OUTPUT_BUFFER, 0);
    }

    return true;
}

void vulkanBufferDestroy(vulkanContext* CONTEXT, vulkanBuffer* BUFFER) 
{
    if (BUFFER->memory) 
    {
        vkFreeMemory(CONTEXT->device.logicalDevice, BUFFER->memory, CONTEXT->allocator);
        BUFFER->memory = 0;
    }
    if (BUFFER->handle) 
    {
        vkDestroyBuffer(CONTEXT->device.logicalDevice, BUFFER->handle, CONTEXT->allocator);
        BUFFER->handle = 0;
    }
    BUFFER->totalSize = 0;
    BUFFER->usage = 0;
    BUFFER->isLocked = false;
}

bool vulkanBufferResize(
    vulkanContext* CONTEXT,
    unsigned long long NEW_SIZE,
    vulkanBuffer* BUFFER,
    VkQueue QUEUE,
    VkCommandPool POOL) 
{
    // Create new buffer.
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = NEW_SIZE;
    bufferInfo.usage = BUFFER->usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VkBuffer newBuffer;
    VK_CHECK(vkCreateBuffer(CONTEXT->device.logicalDevice, &bufferInfo, CONTEXT->allocator, &newBuffer));

    // Gather memory requirements.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(CONTEXT->device.logicalDevice, newBuffer, &requirements);

    // Allocate memory info
    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (unsigned int)BUFFER->memoryIndex;

    // Allocate the memory.
    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(CONTEXT->device.logicalDevice, &allocateInfo, CONTEXT->allocator, &newMemory);
    if (result != VK_SUCCESS) 
    {
        FORGE_LOG_ERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    // Bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, newBuffer, newMemory, 0));

    // Copy over the data
    vulkanBufferCopyTo(CONTEXT, POOL, 0, QUEUE, BUFFER->handle, 0, newBuffer, 0, BUFFER->totalSize);

    // Make sure anything potentially using these is finished.
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);

    // Destroy the old
    if (BUFFER->memory) 
    {
        vkFreeMemory(CONTEXT->device.logicalDevice, BUFFER->memory, CONTEXT->allocator);
        BUFFER->memory = 0;
    }
    if (BUFFER->handle) 
    {
        vkDestroyBuffer(CONTEXT->device.logicalDevice, BUFFER->handle, CONTEXT->allocator);
        BUFFER->handle = 0;
    }

    // Set new properties
    BUFFER->totalSize = NEW_SIZE;
    BUFFER->memory = newMemory;
    BUFFER->handle = newBuffer;

    return true;
}

void vulkanBufferBind(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET) 
{
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, BUFFER->handle, BUFFER->memory, OFFSET));
}

void* vulkanBufferLockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS) 
{
    void* data;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &data));
    return data;
}

void vulkanBufferUnlockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER) 
{
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

void vulkanBufferLoadData(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA) 
{
    void* dataPtr;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &dataPtr));
    forgeCopyMemory(dataPtr, DATA, SIZE);
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

void vulkanBufferCopyTo(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    VkFence FENCE,
    VkQueue QUEUE,
    VkBuffer SOURCE,
    unsigned long long SOURCE_OFFSET,
    VkBuffer DESTINATION,
    unsigned long long DESTINATION_OFFSET,
    unsigned long long SIZE) 
{
    vkQueueWaitIdle(QUEUE);
    // Create a one-time-use command buffer.
    vulkanCommandBuffer tempCommandBuffer;
    vulkanCommandBufferAllocateAndBeginSingleUse(CONTEXT, POOL, &tempCommandBuffer);

    // Prepare the copy command and add it to the command buffer.
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = SOURCE_OFFSET;
    copyRegion.dstOffset = DESTINATION_OFFSET;
    copyRegion.size = SIZE;

    vkCmdCopyBuffer(tempCommandBuffer.handle, SOURCE, DESTINATION, 1, &copyRegion);

    // Submit the buffer for execution and wait for it to complete.
    vulkanCommandBufferEndSingleUse(CONTEXT, POOL, &tempCommandBuffer, QUEUE);
}