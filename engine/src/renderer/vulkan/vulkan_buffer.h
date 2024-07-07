#pragma once
#include "vulkan_types.h"

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_buffer.h
bool createVulkanBuffer(vulkanContext* CONTEXT,
                        unsigned long long SIZE,
                        VkBufferUsageFlagBits USAGE,
                        unsigned int MEMORY_PROPERTY_FLAGS,
                        bool BIND_ON_CREATE,
                        vulkanBuffer* BUFFER);

void destroyVulkanBuffer(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

bool resizeVulkanBuffer(vulkanContext* CONTEXT,
                        unsigned long long NEW_SIZE,
                        vulkanBuffer* BUFFER,
                        VkQueue QUEUE,
                        VkCommandPool COMMAND_POOL);

void bindVulkanBuffer(vulkanContext* CONTEXT,
                      vulkanBuffer* BUFFER,
                      unsigned long long OFFSET);

void* lockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS);

void unlockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

void loadVulkanBufferData(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA);

void copyBuffer(vulkanContext* CONTEXT,
                VkCommandPool POOL,
                VkFence FENCE,
                VkQueue QUEUE,
                VkBuffer SOURCE,
                unsigned long long SOURCE_OFFSET,
                VkBuffer DESTINATION,
                unsigned long long DESTINATION_OFFSET,
                unsigned long long SIZE);

=======
bool vulkanBufferCreate(
    vulkanContext* CONTEXT,
    unsigned long long SIZE,
    VkBufferUsageFlagBits USAGE,
    unsigned int MEMORY_PROPERTY_FLAGS,
    bool BIND_ON_CREATE,
    vulkanBuffer* OUTPUT_BUFFER);

void vulkanBufferDestroy(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

bool vulkanBufferResize(
    vulkanContext* CONTEXT,
    unsigned long long NEW_SIZE,
    vulkanBuffer* BUFFER,
    VkQueue QUEUE,
    VkCommandPool POOL);

void vulkanBufferBind(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET);

void* vulkanBufferLockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS);
void vulkanBufferUnlockMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

void vulkanBufferLoadData(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA);

void vulkanBufferCopyTo(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    VkFence FENCE,
    VkQueue QUEUE,
    VkBuffer SOURCE,
    unsigned long long SOURCE_OFFSET,
    VkBuffer DESTINATION,
    unsigned long long DESTINATION_OFFSET,
    unsigned long long SIZE);
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_buffer.h
