#pragma once
#include "vulkan_types.h"

bool createVulkanBuffer(vulkanContext* CONTEXT, unsigned long long SIZE, VkBufferUsageFlagBits USAGE, unsigned int MEMORY_PROPERTY_FLAGS, bool BIND_ON_CREATE, vulkanBuffer* BUFFER);

void destroyVulkanBuffer(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

void* lockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS);

void unlockBufferMemory(vulkanContext* CONTEXT, vulkanBuffer* BUFFER);

void loadBufferData(vulkanContext* CONTEXT, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, unsigned int FLAGS, const void* DATA);

void copyBuffer(vulkanContext* CONTEXT, VkCommandPool POOL, VkFence FENCE, VkQueue QUEUE, vulkanBu
