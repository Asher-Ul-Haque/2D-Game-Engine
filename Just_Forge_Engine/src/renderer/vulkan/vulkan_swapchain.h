#pragma once
#include "vulkan_types.h"


// - - - Swapchain Functions - - - 

void vulkanSwapchainCreate(
    vulkanContext* CONTEXT,
    unsigned int WIDTH,
    unsigned int height,
    vulkanSwapchain* OUTPUT_SWAPCHAIN);

void vulkanSwapchainRecreate(
    vulkanContext* CONTEXT,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    vulkanSwapchain* SWAPCHAIN);

void vulkanSwapchainDestroy(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN);

bool vulkanSwapchainAcquireNextImageIndex(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN,
    unsigned long long TIMEOUT,
    VkSemaphore IMAGE_AVAILABLE_SEMAPHORES,
    VkFence FENCE,
    unsigned int* OUTPUT_IMAGE_INDEX);

void vulkanSwapchainPresent(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN,
    VkQueue GRAPHICS_QUEUE,
    VkQueue PRESENT_QUEUE,
    VkSemaphore RENDERER_COMPLETE_SEMAPHORE,
    unsigned int PRESENT_IMAGE_INDEX);