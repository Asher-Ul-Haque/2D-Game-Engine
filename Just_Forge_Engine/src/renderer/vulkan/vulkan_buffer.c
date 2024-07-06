#include "vulkan_buffer.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_types.h"
#include "vulkan_utils.h"
#include "core/logger.h"
#include "core/memory.h"


bool createVulkanBuffer(vulkanContext* CONTEXT,
                        unsigned long long SIZE,
                        VkBufferUsageFlagBits USAGE,
                        unsigned int MEMORY_PROPERTY_FLAGS,
                        bool BIND_ON_CREATE,
                        vulkanBuffer* BUFFER)
{
    forgeZeroMemory(BUFFER, sizeof(vulkanBuffer));
    BUFFER->totalSize = SIZE;
    BUFFER->usage = USAGE;
    BUFFER->memoryPropertyFlags = MEMORY_PROPERTY_FLAGS;

    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = SIZE;
    bufferCreateInfo.usage = USAGE;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used by one queue family
    VK_CHECK(vkCreateBuffer(CONTEXT->device.logicalDevice, &bufferCreateInfo, CONTEXT->allocator, &BUFFER->handle));

    //Gather memory requirements
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(CONTEXT->device.logicalDevice, BUFFER->handle, &memoryRequirements);
    BUFFER->memoryIndex = CONTEXT->findMemoryIndex(memoryRequirements.memoryTypeBits, BUFFER->memoryPropertyFlags);
    if (BUFFER->memoryIndex == -1)
    {
        FORGE_LOG_ERROR("Unable to create vulkan buffer because the required memory type index was not found");
        return false;
    }

    // Allocate memory info-
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = (unsigned int) BUFFER->memoryIndex;

    // Allocate memory
    VkResult result = vkAllocateMemory(CONTEXT->device.logicalDevice,
                                       &allocateInfo,
                                       CONTEXT->allocator,
                                       &BUFFER->memory);
    if (result != VK_SUCCESS)
    {
        FORGE_LOG_ERROR("Failed to allocate memory for vulkan buffer");
        return false;
    }

    if (BIND_ON_CREATE)
    {
        bindVulkanBuffer(CONTEXT, BUFFER, 0);
    }

    return true;
}

void destroyVulkanBuffer(vulkanContext* CONTEXT, vulkanBuffer* BUFFER)
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

bool resizeVulkanBuffer(vulkanContext* CONTEXT,
                        unsigned long long NEW_SIZE,
                        vulkanBuffer* BUFFER,
                        VkQueue QUEUE,
                        VkCommandPool COMMAND_POOL)
{
    // Create a new buffer
    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = NEW_SIZE;
    bufferCreateInfo.usage = BUFFER->usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used by one queue family
    
    VkBuffer newBuffer;
    VK_CHECK(vkCreateBuffer(CONTEXT->device.logicalDevice, &bufferCreateInfo, CONTEXT->allocator, &newBuffer));

    // Gather memory requirements
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(CONTEXT->device.logicalDevice, newBuffer, &memoryRequirements);

    // Allocate memory info
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = (unsigned int) BUFFER->memoryIndex;

    // Allocate the memory
    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(CONTEXT->device.logicalDevice, &allocateInfo, CONTEXT->allocator, &newMemory);
    if (result != VK_SUCCESS)
    {
        FORGE_LOG_ERROR("Failed to allocate memory for vulkan buffer");
        return false;
    }

    // Bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, newBuffer, newMemory, 0));

    // Copy the old buffer to the new buffer
    copyBuffer(CONTEXT, COMMAND_POOL, 0, QUEUE, BUFFER->handle, 0, newBuffer, 0, BUFFER->totalSize);

    // Make sure anything potentially using these is finished
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);

    // Destroy the old buffer
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

    // Set the new buffer and memory
    BUFFER->handle = newBuffer;
    BUFFER->memory = newMemory;
    BUFFER->totalSize = NEW_SIZE;

    return true;
}

void bindVulkanBuffer(vulkanContext* CONTEXT,
                      vulkanBuffer* BUFFER,
                      unsigned long long OFFSET)
{
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, BUFFER->handle, BUFFER->memory, OFFSET));
}

void* lockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS)
{
    void* DATA;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &DATA));
    return DATA;
}

void unlockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER)
{
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

void loadVulkanBufferData(vulkanContext *CONTEXT, vulkanBuffer *BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA)
{
    void* dataPtr;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &dataPtr));
    forgeCopyMemory(dataPtr, DATA, SIZE);
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

void copyBuffer(vulkanContext* CONTEXT,
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
    //Create a single use command buffer
    vulkanCommandBuffer tempCommandBuffer;
    commandBufferBeginSingleUse(CONTEXT, POOL, &tempCommandBuffer);

    // Prepare the copy command
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = SOURCE_OFFSET;
    copyRegion.dstOffset = DESTINATION_OFFSET;
    copyRegion.size = SIZE;

    vkCmdCopyBuffer(tempCommandBuffer.handle, SOURCE, DESTINATION, 1, &copyRegion);

    // Submit the command buffer
    commandBufferEndSingleUse(CONTEXT, POOL, &tempCommandBuffer, QUEUE);
}

