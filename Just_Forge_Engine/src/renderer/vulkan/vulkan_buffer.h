#pragma once
#include "vulkan_types.h"

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
