#pragma once
#include "vulkan_types.h"  

void createVulkanSwapchain(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* SWAPCHAIN);

void recreateVulkanSwapchain(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* SWAPCHAIN);

void destroyVulkanSwapchain(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN);

bool vulkanSwapchainAquireNextImageIndex(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN, unsigned long long TIMEOUT, VkSemaphore IMAGE_AVAILABLE_SEMAPHORE, VkFence FENCE, unsigned int* IMAGE_INDEX);

void vulkanSwapchainPresentImage(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN, VkQueue GRAPHICS_QUEUE, VkQueue PRESENT_QUEUE, VkSemaphore RENDER_FINISHED_SEMAPHORE, unsigned int IMAGE_INDEX);
