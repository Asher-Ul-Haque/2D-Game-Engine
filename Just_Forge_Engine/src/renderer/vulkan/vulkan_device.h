#pragma once
#include "vulkan_types.h"

bool vulkanDeviceCreate(vulkanContext* CONTEXT);

void vulkanDeviceDestroy(vulkanContext* CONTEXT);

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice PHYSICAL_DEVICE,
    VkSurfaceKHR SURFACE,
    vulkanSwapchainSupportInfo* OUTPUT_SUPPORT_INFO);

bool vulkanDeviceDetectDepthFormat(vulkanDevice* DEVICE);