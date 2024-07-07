#include "vulkan_buffer.h"
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
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
=======

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
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    }

    return true;
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
void destroyVulkanBuffer(vulkanContext* CONTEXT, vulkanBuffer* BUFFER)
{
    if (BUFFER->memory)
=======
void vulkanBufferDestroy(vulkanContext* CONTEXT, vulkanBuffer* BUFFER) 
{
    if (BUFFER->memory) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    {
        vkFreeMemory(CONTEXT->device.logicalDevice, BUFFER->memory, CONTEXT->allocator);
        BUFFER->memory = 0;
    }
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
    if (BUFFER->handle)
=======
    if (BUFFER->handle) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    {
        vkDestroyBuffer(CONTEXT->device.logicalDevice, BUFFER->handle, CONTEXT->allocator);
        BUFFER->handle = 0;
    }
    BUFFER->totalSize = 0;
    BUFFER->usage = 0;
    BUFFER->isLocked = false;
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
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
=======
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
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
        return false;
    }

    // Bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, newBuffer, newMemory, 0));

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
    // Copy the old buffer to the new buffer
    copyBuffer(CONTEXT, COMMAND_POOL, 0, QUEUE, BUFFER->handle, 0, newBuffer, 0, BUFFER->totalSize);

    // Make sure anything potentially using these is finished
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);

    // Destroy the old buffer
    if (BUFFER->memory)
=======
    // Copy over the data
    vulkanBufferCopyTo(CONTEXT, POOL, 0, QUEUE, BUFFER->handle, 0, newBuffer, 0, BUFFER->totalSize);

    // Make sure anything potentially using these is finished.
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);

    // Destroy the old
    if (BUFFER->memory) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    {
        vkFreeMemory(CONTEXT->device.logicalDevice, BUFFER->memory, CONTEXT->allocator);
        BUFFER->memory = 0;
    }
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
    if (BUFFER->handle)
=======
    if (BUFFER->handle) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    {
        vkDestroyBuffer(CONTEXT->device.logicalDevice, BUFFER->handle, CONTEXT->allocator);
        BUFFER->handle = 0;
    }

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
    // Set the new buffer and memory
    BUFFER->handle = newBuffer;
    BUFFER->memory = newMemory;
    BUFFER->totalSize = NEW_SIZE;
=======
    // Set new properties
    BUFFER->totalSize = NEW_SIZE;
    BUFFER->memory = newMemory;
    BUFFER->handle = newBuffer;
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c

    return true;
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
void bindVulkanBuffer(vulkanContext* CONTEXT,
                      vulkanBuffer* BUFFER,
                      unsigned long long OFFSET)
=======
void vulkanBufferBind(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
{
    VK_CHECK(vkBindBufferMemory(CONTEXT->device.logicalDevice, BUFFER->handle, BUFFER->memory, OFFSET));
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
void* lockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS)
{
    void* DATA;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &DATA));
    return DATA;
}

void unlockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER)
=======
void* vulkanBufferLockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS) 
{
    void* data;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &data));
    return data;
}

void vulkanBufferUnlockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
{
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
void loadVulkanBufferData(vulkanContext *CONTEXT, vulkanBuffer *BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA)
=======
void vulkanBufferLoadData(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA) 
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
{
    void* dataPtr;
    VK_CHECK(vkMapMemory(CONTEXT->device.logicalDevice, BUFFER->memory, OFFSET, SIZE, FLAGS, &dataPtr));
    forgeCopyMemory(dataPtr, DATA, SIZE);
    vkUnmapMemory(CONTEXT->device.logicalDevice, BUFFER->memory);
}

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
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
=======
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
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = SOURCE_OFFSET;
    copyRegion.dstOffset = DESTINATION_OFFSET;
    copyRegion.size = SIZE;

    vkCmdCopyBuffer(tempCommandBuffer.handle, SOURCE, DESTINATION, 1, &copyRegion);

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.c
    // Submit the command buffer
    commandBufferEndSingleUse(CONTEXT, POOL, &tempCommandBuffer, QUEUE);
}

=======
    // Submit the buffer for execution and wait for it to complete.
    vulkanCommandBufferEndSingleUse(CONTEXT, POOL, &tempCommandBuffer, QUEUE);
}
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.c
