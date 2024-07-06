#include "vulkan_fence.h"
#include "core/logger.h"

void createFence(vulkanContext* CONTEXT, bool createSignal, vulkanFence* FENCE)
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = createSignal ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VK_CHECK(vkCreateFence(CONTEXT->device.logicalDevice, &fenceCreateInfo, CONTEXT->allocator, &FENCE->handle));
}

void destroyFence(vulkanContext* CONTEXT, vulkanFence* FENCE)
{
    if (FENCE->handle)
    {
        vkDestroyFence(CONTEXT->device.logicalDevice, FENCE->handle, CONTEXT->allocator);
    FENCE->handle = 0;
    }
    FENCE->isSignaled = false;
}

bool waitForFence(vulkanContext *CONTEXT, vulkanFence *FENCE, unsigned long long TIMEOUT)
{
    if (!FENCE->isSignaled)
    {
        VkResult result = vkWaitForFences(CONTEXT->device.logicalDevice, 1, &FENCE->handle, true, TIMEOUT);

        switch (result)
        {
            case VK_SUCCESS:
                FENCE->isSignaled = true;
                return true;

            case VK_TIMEOUT:
                FORGE_LOG_WARNING("Fence wait timed out");
                break;

            case VK_ERROR_DEVICE_LOST:
                FORGE_LOG_ERROR("Device lost on fence wait");
                break;

            case VK_ERROR_OUT_OF_HOST_MEMORY:
                FORGE_LOG_ERROR("Out of host memory on fence wait");
                break;

            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                FORGE_LOG_ERROR("Out of device memory on fence wait");
                break;

            default:
                FORGE_LOG_ERROR("Unknown error on fence wait");
                break;
        }
    }
    else
    {
        return true;
    }
    return false;
}

void resetFence(vulkanContext *CONTEXT, vulkanFence *FENCE)
{
    if (FENCE->isSignaled)
    {
        VK_CHECK(vkResetFences(CONTEXT->device.logicalDevice, 1, &FENCE->handle));
        FENCE->isSignaled = false;
    }
}
