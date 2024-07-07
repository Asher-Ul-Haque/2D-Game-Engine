#pragma once
#include "vulkan_types.h"

void vulkanCommandBufferAllocate(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    bool IS_PRIMARY,
    vulkanCommandBuffer* OUTPUT_COMMAND_BUFFER);

void vulkanCommandBufferFree(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* COMMAND_BUFFER);

void vulkanCommandBufferBegin(
    vulkanCommandBuffer* COMMAND_BUFFER,
    bool IS_SINGLE_USE,
    bool IS_RENDERPASS_CONTINUE,
    bool IS_SIMULTANEOUS_USE);

void vulkanCommandBufferEnd(vulkanCommandBuffer* COMMAND_BUFFER);

void vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer* COMMAND_BUFFER);

void vulkanCommandBufferReset(vulkanCommandBuffer* COMMAND_BUFFER);

/**
 * Allocates and begins recording to out_command_buffer.
 */
void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* OUTPUT_COMMAND_BUFFER);

/**
 * Ends recording, submits to and waits for queue operation and frees the provided command buffer.
 */
void vulkanCommandBufferEndSingleUse(
    vulkanContext* CONTEXT,
    VkCommandPool POOL,
    vulkanCommandBuffer* COMMAND_BUFFER,
    VkQueue QUEUE);