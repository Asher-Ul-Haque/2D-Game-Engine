#include "vulkan_swapchain.h"

#include "core/logger.h"
#include "core/memory.h"
#include "vulkan_device.h"
#include "vulkan_image.h"


// - - - Vulkan Swapchain Functions - - -

// - - - Pre stating functions
void create(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* OUTPUT_SWAPCHAIN);
void destroy(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN);

// - - - Basic swapchain functions
void vulkanSwapchainCreate(
    vulkanContext* CONTEXT,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    vulkanSwapchain* OUTPUT_SWAPCHAIN)     
{
    // Simply create a new one.
    create(CONTEXT, WIDTH, HEIGHT, OUTPUT_SWAPCHAIN);
}

void vulkanSwapchainRecreate(
    vulkanContext* CONTEXT,
    unsigned int WIDTH,
    unsigned int HEIGHT,
    vulkanSwapchain* SWAPCHAIN) 
{
    // Destroy the old and create a new one.
    destroy(CONTEXT, SWAPCHAIN);
    create(CONTEXT, WIDTH, HEIGHT, SWAPCHAIN);
}

void vulkanSwapchainDestroy(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN) 
{
    destroy(CONTEXT, SWAPCHAIN);
}

bool vulkanSwapchainAcquireNextImageIndex(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN,
    unsigned long long TIMEOUT,
    VkSemaphore IMAGE_AVAILABLE_SEMAPHORE,
    VkFence FENCE,
    unsigned int* OUTPUT_IMAGE_INDEX) 
{
    VkResult result = vkAcquireNextImageKHR(
        CONTEXT->device.logicalDevice,
        SWAPCHAIN->handle,
        TIMEOUT,
        IMAGE_AVAILABLE_SEMAPHORE,
        FENCE,
        OUTPUT_IMAGE_INDEX);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        // Trigger swapchain recreation, then boot out of the render loop.
        vulkanSwapchainRecreate(CONTEXT, CONTEXT->framebufferWidth, CONTEXT->framebufferHeight, SWAPCHAIN);
        return false;
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        FORGE_LOG_FATAL("Failed to acquire swapchain image!");
        return false;
    }

    return true;
}

void vulkanSwapchainPresent(
    vulkanContext* CONTEXT,
    vulkanSwapchain* SWAPCHAIN,
    VkQueue GRAPHICS_QUEUE,
    VkQueue PRESENT_QUEUE,
    VkSemaphore RENDERER_COMPLETE_SEMAPHORE,
    unsigned int PRESENT_IMAGE_INDEX) 
{
    // Return the image to the swapchain for presentation.
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &RENDERER_COMPLETE_SEMAPHORE;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &SWAPCHAIN->handle;
    presentInfo.pImageIndices = &PRESENT_IMAGE_INDEX;
    presentInfo.pResults = 0;

    VkResult result = vkQueuePresentKHR(PRESENT_QUEUE, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
    {
        // Swapchain is out of date, suboptimal or a framebuffer resize has occurred. Trigger swapchain recreation.
        vulkanSwapchainRecreate(CONTEXT, CONTEXT->framebufferWidth, CONTEXT->framebufferHeight, SWAPCHAIN);
    } 
    else if (result != VK_SUCCESS) 
    {
        FORGE_LOG_FATAL("Failed to present swap chain image!");
    }

    // Increment (and loop) the index.
    CONTEXT->currentFrame = (CONTEXT->currentFrame + 1) % SWAPCHAIN->maxFramesInFlight;
}


// - - - Implementation of pre implemented functions

void create(vulkanContext* CONTEXT, unsigned int WIDTH, unsigned int HEIGHT, vulkanSwapchain* OUTPUT_SWAPCHAIN) 
{
    VkExtent2D swapchainExtent = {WIDTH, HEIGHT};

    // Choose a swap surface format.
    bool found = false;
    for (unsigned int i = 0; i < CONTEXT->device.swapchainSupport.formatCount; ++i) 
    {
        VkSurfaceFormatKHR format = CONTEXT->device.swapchainSupport.formats[i];
        // Preferred formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            OUTPUT_SWAPCHAIN->imageFormat = format;
            found = true;
            break;
        }
    }

    if (!found) 
    {
        OUTPUT_SWAPCHAIN->imageFormat = CONTEXT->device.swapchainSupport.formats[0];
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

    // Requery swapchain support.
    vulkanDeviceQuerySwapchainSupport(
        CONTEXT->device.physicalDevice,
        CONTEXT->surface,
        &CONTEXT->device.swapchainSupport);

    // Swapchain extent
    if (CONTEXT->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) 
    {
        swapchainExtent = CONTEXT->device.swapchainSupport.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU.
    VkExtent2D min = CONTEXT->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = CONTEXT->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = FORGE_CLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = FORGE_CLAMP(swapchainExtent.height, min.height, max.height);

    unsigned int imageCount = CONTEXT->device.swapchainSupport.capabilities.minImageCount + 1;
    if (CONTEXT->device.swapchainSupport.capabilities.maxImageCount > 0 && imageCount > CONTEXT->device.swapchainSupport.capabilities.maxImageCount) 
    {
        imageCount = CONTEXT->device.swapchainSupport.capabilities.maxImageCount;
    }

    OUTPUT_SWAPCHAIN->maxFramesInFlight = imageCount - 1;

    // Swapchain create info
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface = CONTEXT->surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = OUTPUT_SWAPCHAIN->imageFormat.format;
    swapchainCreateInfo.imageColorSpace = OUTPUT_SWAPCHAIN->imageFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (CONTEXT->device.graphicsQueueIndex != CONTEXT->device.presentQueueIndex) 
    {
        unsigned int queueFamilyIndices[] = {
            (unsigned int)CONTEXT->device.graphicsQueueIndex,
            (unsigned int)CONTEXT->device.presentQueueIndex};
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else 
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = 0;
    }

    swapchainCreateInfo.preTransform = CONTEXT->device.swapchainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(CONTEXT->device.logicalDevice, &swapchainCreateInfo, CONTEXT->allocator, &OUTPUT_SWAPCHAIN->handle));

    // Start with a zero frame index.
    CONTEXT->currentFrame = 0;

    // Images
    OUTPUT_SWAPCHAIN->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(CONTEXT->device.logicalDevice, OUTPUT_SWAPCHAIN->handle, &OUTPUT_SWAPCHAIN->imageCount, 0));
    if (!OUTPUT_SWAPCHAIN->images) 
    {
        OUTPUT_SWAPCHAIN->images = (VkImage*)forgeAllocateMemory(sizeof(VkImage) * OUTPUT_SWAPCHAIN->imageCount, MEMORY_TAG_RENDERER);
    }
    if (!OUTPUT_SWAPCHAIN->views) 
    {
        OUTPUT_SWAPCHAIN->views = (VkImageView*)forgeAllocateMemory(sizeof(VkImageView) * OUTPUT_SWAPCHAIN->imageCount, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(CONTEXT->device.logicalDevice, OUTPUT_SWAPCHAIN->handle, &OUTPUT_SWAPCHAIN->imageCount, OUTPUT_SWAPCHAIN->images));

    // Views
    for (unsigned int i = 0; i < OUTPUT_SWAPCHAIN->imageCount; ++i) 
    {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = OUTPUT_SWAPCHAIN->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = OUTPUT_SWAPCHAIN->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(CONTEXT->device.logicalDevice, &viewInfo, CONTEXT->allocator, &OUTPUT_SWAPCHAIN->views[i]));
    }

    // Depth resources
    if (!vulkanDeviceDetectDepthFormat(&CONTEXT->device)) 
    {
        CONTEXT->device.depthFormat = VK_FORMAT_UNDEFINED;
        FORGE_LOG_FATAL("Failed to find a supported format!");
    }

    // Create depth image and its view.
    vulkanImageCreate(
        CONTEXT,
        VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        CONTEXT->device.depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &OUTPUT_SWAPCHAIN->depthAttachment);

    FORGE_LOG_INFO("Swapchain created successfully.");
}

void destroy(vulkanContext* CONTEXT, vulkanSwapchain* SWAPCHAIN) 
{
    vkDeviceWaitIdle(CONTEXT->device.logicalDevice);
    vulkanImageDestroy(CONTEXT, &SWAPCHAIN->depthAttachment);

    // Only destroy the views, not the images, since those are owned by the swapchain and are thus
    // destroyed when it is.
    for (unsigned int i = 0; i < SWAPCHAIN->imageCount; ++i) 
    {
        vkDestroyImageView(CONTEXT->device.logicalDevice, SWAPCHAIN->views[i], CONTEXT->allocator);
    }

    vkDestroySwapchainKHR(CONTEXT->device.logicalDevice, SWAPCHAIN->handle, CONTEXT->allocator);
}