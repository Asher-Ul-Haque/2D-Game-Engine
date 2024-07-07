#include "vulkan_image.h"

#include "vulkan_device.h"

#include "core/memory.h"
#include "core/logger.h"

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
    vulkanImage* OUTPUT_IMAGE) 
{
    // Copy params
    OUTPUT_IMAGE->width = WIDTH;
    OUTPUT_IMAGE->height = HEIGHT;

    // Creation info.
    VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = WIDTH;
    image_create_info.extent.height = HEIGHT;
    image_create_info.extent.depth = 1;  // TODO: Support configurable depth.
    image_create_info.mipLevels = 4;     // TODO: Support mip mapping
    image_create_info.arrayLayers = 1;   // TODO: Support number of layers in the image.
    image_create_info.format = FORMAT;
    image_create_info.tiling = TILING;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = USAGE;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;          // TODO: Configurable sample count.
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.

    VK_CHECK(vkCreateImage(CONTEXT->device.logicalDevice, &image_create_info, CONTEXT->allocator, &OUTPUT_IMAGE->handle));

    // Query memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(CONTEXT->device.logicalDevice, OUTPUT_IMAGE->handle, &memoryRequirements);

    int memoryType = CONTEXT->findMemoryIndex(memoryRequirements.memoryTypeBits, MEMORY_FLAGS);
    if (memoryType == -1) 
    {
        FORGE_LOG_ERROR("Required memory type not found. Image not valid.");
    }

    // Allocate memory
    VkMemoryAllocateInfo memoryAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(CONTEXT->device.logicalDevice, &memoryAllocInfo, CONTEXT->allocator, &OUTPUT_IMAGE->memory));

    // Bind the memory
    VK_CHECK(vkBindImageMemory(CONTEXT->device.logicalDevice, OUTPUT_IMAGE->handle, OUTPUT_IMAGE->memory, 0));  // TODO: configurable memory offset.

    // Create view
    if (CREATE_VIEW) 
    {
        OUTPUT_IMAGE->view = 0;
        vulkanImageViewCreate(CONTEXT, FORMAT, OUTPUT_IMAGE, VIEW_ASPECT_FLAGS);
    }
}

void vulkanImageViewCreate(
    vulkanContext* CONTEXT,
    VkFormat FORMAT,
    vulkanImage* IMAGE,
    VkImageAspectFlags ASPECT_FLAGS) 
{
    VkImageViewCreateInfo viewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = IMAGE->handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO: Make configurable.
    viewCreateInfo.format = FORMAT;
    viewCreateInfo.subresourceRange.aspectMask = ASPECT_FLAGS;

    // TODO: Make configurable
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(CONTEXT->device.logicalDevice, &viewCreateInfo, CONTEXT->allocator, &IMAGE->view));
}

void vulkanImageDestroy(vulkanContext* CONTEXT, vulkanImage* IMAGE) 
{
    if (IMAGE->view) 
    {
        vkDestroyImageView(CONTEXT->device.logicalDevice, IMAGE->view, CONTEXT->allocator);
        IMAGE->view = 0;
    }
    if (IMAGE->memory) 
    {
        vkFreeMemory(CONTEXT->device.logicalDevice, IMAGE->memory, CONTEXT->allocator);
        IMAGE->memory = 0;
    }
    if (IMAGE->handle) 
    {
        vkDestroyImage(CONTEXT->device.logicalDevice, IMAGE->handle, CONTEXT->allocator);
        IMAGE->handle = 0;
    }
}