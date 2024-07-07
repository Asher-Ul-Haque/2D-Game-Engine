#include "vulkan_fence.h"
#include "core/logger.h"

void vulkanFenceCreate(
    vulkanContext* CONTEXT,
    bool CREATE_SIGNAL,
    vulkanFence* OUTPUT_FENCE) 
{
    // Make sure to signal the fence if required.
    OUTPUT_FENCE->isSignaled = CREATE_SIGNAL;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (OUTPUT_FENCE->isSignaled) 
    {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        CONTEXT->device.logicalDevice,
        &fenceCreateInfo,
        CONTEXT->allocator,
        &OUTPUT_FENCE->handle));
}

void vulkanFenceDestroy(vulkanContext* CONTEXT, vulkanFence* FENCE) 
{
    if (FENCE->handle) 
    {
        vkDestroyFence(
            CONTEXT->device.logicalDevice,
            FENCE->handle,
            CONTEXT->allocator);
        FENCE->handle = 0;
    }
    FENCE->isSignaled = false;
}

bool vulkanFenceWait(vulkanContext* CONTEXT, vulkanFence* FENCE, unsigned long long TIMEOUT) 
{
    if (!FENCE->isSignaled) 
    {
        VkResult result = vkWaitForFences(
            CONTEXT->device.logicalDevice,
            1,
            &FENCE->handle,
            true,
            TIMEOUT);
        switch (result) 
        {
            case VK_SUCCESS:
                FENCE->isSignaled = true;
                return true;
            case VK_TIMEOUT:
                FORGE_LOG_WARNING("vk_fence_wait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                FORGE_LOG_ERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                FORGE_LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                FORGE_LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                break;
            default:
                FORGE_LOG_ERROR("vk_fence_wait - An unknown error has occurred.");
                break;
        }
    } 
    else 
    {
        // If already signaled, do not wait.
        return true;
    }

    return false;
}

void vulkanFenceReset(vulkanContext* CONTEXT, vulkanFence* FENCE) 
{
    if (FENCE->isSignaled) 
    {
        VK_CHECK(vkResetFences(CONTEXT->device.logicalDevice, 1, &FENCE->handle));
        FENCE->isSignaled = false;
    }
}