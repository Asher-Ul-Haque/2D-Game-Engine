#include "vulkan_swapchain.h"
#include "core/logger.h"
#include "core/memory.h"
#include "vulkan_device.h"
#include "vulkan_types.h"
#include "vulkan_image.h"
#include <vulkan/vulkan_core.h>

void create(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* SWAPCHAIN)
{
    VkExtent2D swapchainExtent = {WIDTH, HEIGHT};
    SWAPCHAIN->maxFramesInFlight = 2; //suporting triple buffering
    
    // Choose a swap surface format
    bool foundFormat = false;
    for (unsigned int i = 0; i < CONTEXT->device.swapchainSupport.formatCount; ++i)
    {
        VkSurfaceFormatKHR format = CONTEXT->device.swapchainSupport.formats[i];
        // Prefered format
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SWAPCHAIN->imageFormat = format;
            foundFormat = true;
            break;
        }
    }
    if (!foundFormat)
    {
        SWAPCHAIN->imageFormat = CONTEXT->device.swapchainSupport.formats[0];
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (unsigned int i = 0; i < CONTEXT->device.swapchainSupport.presentModeCount; ++i)
    {
        VkPresentModeKHR mode = CONTEXT->device.swapchainSupport.presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
        }
    }

    //Requery swapchain support
    vulkanDeviceQuerySwapchainSupport(CONTEXT->device.physicalDevice, CONTEXT->surface, &CONTEXT->device.swapchainSupport);

    if (CONTEXT->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX)
    {
        swapchainExtent = CONTEXT->device.swapchainSupport.capabilities.currentExtent;
    }

    //Clamp to the value allowed by the GPU
    VkExtent2D min = CONTEXT->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = CONTEXT->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = FORGE_CLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = FORGE_CLAMP(swapchainExtent.height, min.height, max.height);

    unsigned int imageCount = CONTEXT->device.swapchainSupport.capabilities.minImageCount + 1;
    if (CONTEXT->device.swapchainSupport.capabilities.maxImageCount > 0 && imageCount > CONTEXT->device.swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = CONTEXT->device.swapchainSupport.capabilities.maxImageCount;
    }
    
    // Swapchain create info
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = CONTEXT->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = SWAPCHAIN->imageFormat.format;
    createInfo.imageColorSpace = SWAPCHAIN->imageFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue families
    if (CONTEXT->device.graphicsQueueIndex != CONTEXT->device.presentQueueIndex)
    {
        unsigned int queueFamilyIndices[] = {
            (unsigned int)CONTEXT->device.graphicsQueueIndex, 
            (unsigned int)CONTEXT->device.presentQueueIndex};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;
    }

    createInfo.preTransform = CONTEXT->device.swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(CONTEXT->device.logicalDevice, &createInfo, CONTEXT->allocator, &SWAPCHAIN->handle));

    //Get the swapchain images
    CONTEXT->currentFrame = 0;
    SWAPCHAIN->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(CONTEXT->device.logicalDevice, SWAPCHAIN->handle, &SWAPCHAIN->imageCount, 0));

    if (!SWAPCHAIN->images)
    {
        SWAPCHAIN->images = forgeAllocateMemory(SWAPCHAIN->imageCount * sizeof(VkImage), MEMORY_TAG_RENDERER);
    }
    if (!SWAPCHAIN->imageViews)
    {
        SWAPCHAIN->imageViews = forgeAllocateMemory(SWAPCHAIN->imageCount * sizeof(VkImageView), MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(CONTEXT->device.logicalDevice, SWAPCHAIN->handle, &SWAPCHAIN->imageCount, SWAPCHAIN->images));

    //Create image views
    for (unsigned int i = 0; i < SWAPCHAIN->imageCount; ++i)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = SWAPCHAIN->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = SWAPCHAIN->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(CONTEXT->device.logicalDevice, &viewInfo, CONTEXT->allocator, &SWAPCHAIN->imageViews[i]));
    }

    // Depth resources
    if (!vulkanDeviceDetectDepthFormat(&CONTEXT->device))
    {
        CONTEXT->device.depthFormat = VK_FORMAT_UNDEFINED;
        FORGE_LOG_FATAL("Failed to detect depth format");
    }

    // Create depth image and its view
    createVulkanImage(CONTEXT, VK_IMAGE_TYPE_2D, swapchainExtent.width, swapchainExtent.height, CONTEXT->device.depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_DEPTH_BIT, &SWAPCHAIN->depthAttachment);

    FORGE_LOG_INFO("Swapchain created");
}

void destroy(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN)
{
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);
    destroyVulkanImage(CONTEXT, &SWAPCHAIN->depthAttachment);

    //Only destroy the views, not the images since they are owned by the swapchain
    for (unsigned int i = 0; i < SWAPCHAIN->imageCount; ++i)
    {
        vkDestroyImageView(CONTEXT->device.logicalDevice, SWAPCHAIN->imageViews[i], CONTEXT->allocator);
    }

    vkDestroySwapchainKHR(CONTEXT->device.logicalDevice, SWAPCHAIN->handle, CONTEXT->allocator);
}


void createVulkanSwapchain(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* SWAPCHAIN)
{
    create(CONTEXT, WIDTH, HEIGHT, SWAPCHAIN);
}

void recreateVulkanSwapchain(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* SWAPCHAIN)
{
    destroy(CONTEXT, SWAPCHAIN);
    create(CONTEXT, WIDTH, HEIGHT, SWAPCHAIN);
}

void destroyVulkanSwapchain(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN)
{
    destroy(CONTEXT, SWAPCHAIN);
}

bool vulkanSwapchainAquireNextImageIndex(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN, unsigned long long TIMEOUT, VkSemaphore IMAGE_AVAILABLE_SEMAPHORE, VkFence FENCE, unsigned int* IMAGE_INDEX)
{
    VkResult result = vkAcquireNextImageKHR(CONTEXT->device.logicalDevice, SWAPCHAIN->handle, TIMEOUT, IMAGE_AVAILABLE_SEMAPHORE, FENCE, IMAGE_INDEX);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        //Trigger swapchain recreation and boot out of render order
        recreateVulkanSwapchain(CONTEXT, CONTEXT->framebufferWidth, CONTEXT->framebufferHeight, SWAPCHAIN);
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        FORGE_LOG_FATAL("Failed to aquire swapchain image");
        return false;
    }
    return true;
}

void vulkanSwapchainPresentImage(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN, VkQueue GRAPHICS_QUEUE, VkQueue PRESENT_QUEUE, VkSemaphore RENDER_FINISHED_SEMAPHORE, unsigned int IMAGE_INDEX)
{
    //Return the image back to the swapchain for presentation
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pWaitSemaphores = &RENDER_FINISHED_SEMAPHORE;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &SWAPCHAIN->handle;
    presentInfo.pImageIndices = &IMAGE_INDEX;
    presentInfo.pResults = 0;

    VkResult result = vkQueuePresentKHR(PRESENT_QUEUE, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        //Trigger swapchain recreation
        recreateVulkanSwapchain(CONTEXT, CONTEXT->framebufferWidth, CONTEXT->framebufferHeight, SWAPCHAIN);
    }
    else if (result != VK_SUCCESS)
    {
        FORGE_LOG_FATAL("Failed to present swapchain image");
    }

    //Increment the frame index
    CONTEXT->currentFrame = (CONTEXT->currentFrame + 1) % SWAPCHAIN->maxFramesInFlight;
}
