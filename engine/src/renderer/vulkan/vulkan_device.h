#pragma once
#include "vulkan_types.h"


// - - - | Vulkan Device Functions | - - -

bool createVulkanDevice(vulkanContext* CONTEXT);

void destroyVulkanDevice(vulkanContext* CONTEXT);

bool vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice GPU, VkSurfaceKHR SURFACE, vulkanSwapchainSupportInfo* INFO);

bool vulkanDeviceDetectDepthFormat(vulkanDevice* DEVICE);
