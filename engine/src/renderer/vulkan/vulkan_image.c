#include "vulkan_image.h"
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/memory.h"

void createVulkanImage(vulkanContext *CONTEXT, VkImageType IMAGE_TYPE, unsigned int WIDTH, unsigned int HEIGHT, VkFormat FORMAT, VkImageTiling TILING, VkImageUsageFlags USAGE, VkMemoryPropertyFlags MEMORY_FLAGS, bool32 CREATE_VIEW, VkImageAspectFlags VIEW_ASPECT_FLAGS, vulkanImage *OUT_IMAGE)
{
    //Copy parameters
    OUT_IMAGE->width = WIDTH;
    OUT_IMAGE->height = HEIGHT;

    //Creation info
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = WIDTH;
    imageInfo.extent.height = HEIGHT;
    imageInfo.extent.depth = 1; // TODO: configurable depth
    imageInfo.mipLevels = 4;  // TODO: support mipmapping
    imageInfo.arrayLayers = 1; // TODO: support multiple array layers
    imageInfo.format = FORMAT;
    imageInfo.tiling = TILING;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = USAGE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: configurable sample count
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: configurable sharing mode

    VK_CHECK(vkCreateImage(CONTEXT->device.logicalDevice, &imageInfo, CONTEXT->allocator, &OUT_IMAGE->handle));

    //Memory requirements
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(CONTEXT->device.logicalDevice, OUT_IMAGE->handle, &memoryRequirements);

    int memoryType = CONTEXT->findMemoryIndex(memoryRequirements.memoryTypeBits, MEMORY_FLAGS);
    if (memoryType == -1)
    {
        FORGE_LOG_ERROR("Failed to find suitable memory type");
    }

    //Allocate memory
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(CONTEXT->device.logicalDevice, &allocateInfo, CONTEXT->allocator, &OUT_IMAGE->memory));

    // Bind the memory
    VK_CHECK(vkBindImageMemory(CONTEXT->device.logicalDevice, OUT_IMAGE->handle, OUT_IMAGE->memory, 0));

    //Create image view
    if (CREATE_VIEW)
    {
        OUT_IMAGE->view = 0;
        createImageView(CONTEXT, FORMAT, OUT_IMAGE, VIEW_ASPECT_FLAGS);
    }
}

void createImageView(vulkanContext *CONTEXT, VkFormat FORMAT, vulkanImage *IMAGE, VkImageAspectFlags ASPECT_FLAGS)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = IMAGE->handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //TODO: make this configurable
    viewInfo.format = FORMAT;
    viewInfo.subresourceRange.aspectMask = ASPECT_FLAGS;

    //TODO: make this configurable
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(CONTEXT->device.logicalDevice, &viewInfo, CONTEXT->allocator, &IMAGE->view));
}

void destroyVulkanImage(vulkanContext *CONTEXT, vulkanImage *IMAGE)
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
