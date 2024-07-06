#pragma once
#include "vulkan_types.h"

void vulkanImageCreate(
    vulkanContext* CONTEXT,
    VkImageType IMAGE_TYPE,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    VkFormat FORMAT,
    VkImageTiling TILING,
    VkImageUsageFlags USAGE,
    VkMemoryPropertyFlags MEMORY_FLAGS,
    boolean CREATE_VIEW,
    VkImageAspectFlags VIEW_ASPECT_FLAGS,
    vulkanImage* OUTPUT_IMAGE);

void vulkanImageViewCreate(
    vulkanContext* CONTEXT,
    VkFormat FORMAT,
    vulkanImage* IMAGE,
    VkImageAspectFlags ASPECT_FLAGS);

void vulkanImageDestroy(vulkanContext* CONTEXT, vulkanImage* IMAGE);
