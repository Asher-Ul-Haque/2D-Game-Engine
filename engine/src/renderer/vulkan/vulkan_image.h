#pragma once
#include "vulkan_types.h"
#include <vulkan/vulkan_core.h>

void createVulkanImage(vulkanContext* CONTEXT, VkImageType IMAGE_TYPE, unsigned int WIDTH, unsigned int HEIGHT, VkFormat FORMAT, VkImageTiling TILING, VkImageUsageFlags USAGE, VkMemoryPropertyFlags MEMORY_FLAGS, bool32 CREATE_VIEW, VkImageAspectFlags VIEW_ASPECT_FLAGS, vulkanImage* OUT_IMAGE);

void createImageView(vulkanContext* CONTEXT, VkFormat FORMAT, vulkanImage* IMAGE, VkImageAspectFlags ASPECT_FLAGS);

void destroyVulkanImage(vulkanContext* CONTEXT, vulkanImage* IMAGE);
