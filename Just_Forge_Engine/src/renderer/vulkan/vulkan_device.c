#include "vulkan_device.h"
#include "core/logger.h"

#include <string.h>

#include "core/memory.h"
#include "dataStructures/list.h"

typedef struct vulkanPhysicalDeviceRequirements 
{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    // list
    const char** deviceExtensionNames;
    bool samplerAnisotropy;
    bool discreteGpu;
} vulkanPhysicalDeviceRequirements;

typedef struct VulkanPhysicalDeviceQueueFamilyInfo 
{
    unsigned int graphicsFamilyIndex;
    unsigned int presentFamilyIndex;
    unsigned int computeFamilyIndex;
    unsigned int transferFamilyIndex;
} VulkanPhysicalDeviceQueueFamilyInfo;

bool selectPhysicalDevice(vulkanContext* CONTEXT);
bool physicalDeviceMeetsRequirements(
    VkPhysicalDevice DEVICE,
    VkSurfaceKHR SURFACE,
    const VkPhysicalDeviceProperties* PROPERTIES,
    const VkPhysicalDeviceFeatures* FEATURES,
    const vulkanPhysicalDeviceRequirements* REQUIREMENTS,
    VulkanPhysicalDeviceQueueFamilyInfo* OUTPUT_QUEUE_FAMILY_INFO,
    vulkanSwapchainSupportInfo* OUTPUT_SWAPCHAIN_SUPPORT);

bool vulkanDeviceCreate(vulkanContext* CONTEXT) 
{
    if (!selectPhysicalDevice(CONTEXT)) 
    {
        return false;
    }

    FORGE_LOG_INFO("Creating logical device...");
    // NOTE: Do not create additional queues for shared indices.
    bool presentSharesGraphicsQueue = CONTEXT->device.graphicsQueueIndex == CONTEXT->device.presentQueueIndex;
    bool transferSharesGraphicsQueue = CONTEXT->device.graphicsQueueIndex == CONTEXT->device.transferQueueIndex;
    unsigned int indexCount = 1;
    if (!presentSharesGraphicsQueue) 
    {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue) 
    {
        indexCount++;
    }
    unsigned int indices[32];
    unsigned char index = 0;
    indices[index++] = CONTEXT->device.graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) 
    {
        indices[index++] = CONTEXT->device.presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue) 
    {
        indices[index++] = CONTEXT->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[32];
    for (unsigned int i = 0; i < indexCount; ++i) 
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;

        // TODO: Enable this for a future enhancement.
        // if (indices[i] == context->device.graphicsQueueIndex) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        float queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    bool portabilityRequired = false;
    unsigned int availableExtensionCount = 0;
    VkExtensionProperties* availableExtensions = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(CONTEXT->device.physicalDevice, 0, &availableExtensionCount, 0));
    if (availableExtensionCount != 0) 
    {
        availableExtensions = forgeAllocateMemory(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(CONTEXT->device.physicalDevice, 0, &availableExtensionCount, availableExtensions));
        for (unsigned int i = 0; i < availableExtensionCount; ++i) 
        {
            if (strcmp(availableExtensions[i].extensionName, "VK_KHR_portability_subset") == 0) 
            {
                FORGE_LOG_INFO("Adding required extension 'VK_KHR_portability_subset'.");
                portabilityRequired = true;
                break;
            }
        }
    }
    forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);

    unsigned int extensionCount = portabilityRequired ? 2 : 1;
    const char** extensionNames = portabilityRequired
            ? (const char* [2]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset" }
            : (const char* [1]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = extensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames;

    // Deprecated and ignored, so pass nothing.
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;

    // Create the device.
    VK_CHECK(vkCreateDevice(
        CONTEXT->device.physicalDevice,
        &deviceCreateInfo,
        CONTEXT->allocator,
        &CONTEXT->device.logicalDevice));

    FORGE_LOG_INFO("Logical device created.");

    // Get queues.
    vkGetDeviceQueue(
        CONTEXT->device.logicalDevice,
        CONTEXT->device.graphicsQueueIndex,
        0,
        &CONTEXT->device.graphicsQueue);

    vkGetDeviceQueue(
        CONTEXT->device.logicalDevice,
        CONTEXT->device.presentQueueIndex,
        0,
        &CONTEXT->device.presentQueue);

    vkGetDeviceQueue(
        CONTEXT->device.logicalDevice,
        CONTEXT->device.transferQueueIndex,
        0,
        &CONTEXT->device.transferQueue);
    FORGE_LOG_INFO("Queues obtained.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo poolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = CONTEXT->device.graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        CONTEXT->device.logicalDevice,
        &poolCreateInfo,
        CONTEXT->allocator,
        &CONTEXT->device.graphicsCommandPool));
    FORGE_LOG_INFO("Graphics command pool created.");

    return true;
}

void vulkanDeviceDestroy(vulkanContext* CONTEXT) 
{
    // Unset queues
    CONTEXT->device.graphicsQueue = 0;
    CONTEXT->device.presentQueue = 0;
    CONTEXT->device.transferQueue = 0;

    FORGE_LOG_INFO("Destroying command pools...");
    vkDestroyCommandPool(
        CONTEXT->device.logicalDevice,
        CONTEXT->device.graphicsCommandPool,
        CONTEXT->allocator);

    // Destroy logical device
    FORGE_LOG_INFO("Destroying logical device...");
    if (CONTEXT->device.logicalDevice) 
    {
        vkDestroyDevice(CONTEXT->device.logicalDevice, CONTEXT->allocator);
        CONTEXT->device.logicalDevice = 0;
    }

    // Physical devices are not destroyed.
    FORGE_LOG_INFO("Releasing physical device resources...");
    CONTEXT->device.physicalDevice = 0;

    if (CONTEXT->device.swapchainSupport.formats) 
    {
        forgeFreeMemory(
            CONTEXT->device.swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * CONTEXT->device.swapchainSupport.formatCount,
            MEMORY_TAG_RENDERER);
        CONTEXT->device.swapchainSupport.formats = 0;
        CONTEXT->device.swapchainSupport.formatCount = 0;
    }

    if (CONTEXT->device.swapchainSupport.presentModes) 
    {
        forgeFreeMemory(
            CONTEXT->device.swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * CONTEXT->device.swapchainSupport.presentModeCount,
            MEMORY_TAG_RENDERER);
        CONTEXT->device.swapchainSupport.presentModes = 0;
        CONTEXT->device.swapchainSupport.presentModeCount = 0;
    }

    forgeZeroMemory(
        &CONTEXT->device.swapchainSupport.capabilities,
        sizeof(CONTEXT->device.swapchainSupport.capabilities));

    CONTEXT->device.graphicsQueueIndex = -1;
    CONTEXT->device.presentQueueIndex = -1;
    CONTEXT->device.transferQueueIndex = -1;
}

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice PHYSICAL_DEVICE,
    VkSurfaceKHR SURFACE,
    vulkanSwapchainSupportInfo* OUTPUT_SUPPORT_INFO) 
{
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        PHYSICAL_DEVICE,
        SURFACE,
        &OUTPUT_SUPPORT_INFO->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        PHYSICAL_DEVICE,
        SURFACE,
        &OUTPUT_SUPPORT_INFO->formatCount,
        0));

    if (OUTPUT_SUPPORT_INFO->formatCount != 0) 
    {
        if (!OUTPUT_SUPPORT_INFO->formats) 
        {
            OUTPUT_SUPPORT_INFO->formats = forgeAllocateMemory(sizeof(VkSurfaceFormatKHR) * OUTPUT_SUPPORT_INFO->formatCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            PHYSICAL_DEVICE,
            SURFACE,
            &OUTPUT_SUPPORT_INFO->formatCount,
            OUTPUT_SUPPORT_INFO->formats));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        PHYSICAL_DEVICE,
        SURFACE,
        &OUTPUT_SUPPORT_INFO->presentModeCount,
        0));
    if (OUTPUT_SUPPORT_INFO->presentModeCount != 0) 
    {
        if (!OUTPUT_SUPPORT_INFO->presentModes) 
        {
            OUTPUT_SUPPORT_INFO->presentModes = forgeAllocateMemory(sizeof(VkPresentModeKHR) * OUTPUT_SUPPORT_INFO->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            PHYSICAL_DEVICE,
            SURFACE,
            &OUTPUT_SUPPORT_INFO->presentModeCount,
            OUTPUT_SUPPORT_INFO->presentModes));
    }
}

bool vulkanDeviceDetectDepthFormat(vulkanDevice* DEVICE) 
{
    // Format candidates
    const unsigned long long candidateCount = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    unsigned int flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (unsigned long long i = 0; i < candidateCount; ++i) 
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(DEVICE->physicalDevice, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) 
        {
            DEVICE->depthFormat = candidates[i];
            return true;
        } 
        else if ((properties.optimalTilingFeatures & flags) == flags) 
        {
            DEVICE->depthFormat = candidates[i];
            return true;
        }
    }

    return false;
}

bool selectPhysicalDevice(vulkanContext* CONTEXT) 
{
    unsigned int physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(CONTEXT->instance, &physicalDeviceCount, 0));
    if (physicalDeviceCount == 0) 
    {
        FORGE_LOG_FATAL("No devices which support Vulkan were found.");
        return false;
    }
    const unsigned int maxDeviceCount = 32;
    VkPhysicalDevice physical_devices[maxDeviceCount];
    VK_CHECK(vkEnumeratePhysicalDevices(CONTEXT->instance, &physicalDeviceCount, physical_devices));
    for (unsigned int i = 0; i < physicalDeviceCount; ++i) 
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        // TODO: These requirements should probably be driven by engine
        // configuration.
        vulkanPhysicalDeviceRequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = true;
        requirements.samplerAnisotropy = true;
    #if FORGE_PLATFORM_APPLE
        requirements.discrete_gpu = false;
    #else
        requirements.discreteGpu = false;
    #endif
        requirements.deviceExtensionNames = listCreate(const char*);
        listAppend(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceQueueFamilyInfo queueInfo = {};
        bool result = physicalDeviceMeetsRequirements(
            physical_devices[i],
            CONTEXT->surface,
            &properties,
            &features,
            &requirements,
            &queueInfo,
            &CONTEXT->device.swapchainSupport);

        if (result) 
        {
            FORGE_LOG_INFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) 
            {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    FORGE_LOG_INFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    FORGE_LOG_INFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    FORGE_LOG_INFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    FORGE_LOG_INFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    FORGE_LOG_INFO("GPU type is CPU.");
                    break;
            }

            FORGE_LOG_INFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            // Vulkan API version.
            FORGE_LOG_INFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            // Memory information
            for (unsigned int j = 0; j < memory.memoryHeapCount; ++j) 
            {
                float memorySizeGB = (((float)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) 
                {
                    FORGE_LOG_INFO("Local GPU memory: %.2f GB", memorySizeGB);
                } 
                else 
                {
                    FORGE_LOG_INFO("Shared System memory: %.2f GB", memorySizeGB);
                }
            }

            CONTEXT->device.physicalDevice = physical_devices[i];
            CONTEXT->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            CONTEXT->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            CONTEXT->device.transferQueueIndex = queueInfo.transferFamilyIndex;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            CONTEXT->device.properties = properties;
            CONTEXT->device.features = features;
            CONTEXT->device.memory = memory;
            break;
        }
    }

    // Ensure a device was selected
    if (!CONTEXT->device.physicalDevice) 
    {
        FORGE_LOG_ERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    FORGE_LOG_INFO("Physical device selected.");
    return true;
}

bool physicalDeviceMeetsRequirements(
    VkPhysicalDevice DEVICE,
    VkSurfaceKHR SURFACE,
    const VkPhysicalDeviceProperties* PROPERTIES,
    const VkPhysicalDeviceFeatures* FEATURES,
    const vulkanPhysicalDeviceRequirements* REQUIREMENTS,
    VulkanPhysicalDeviceQueueFamilyInfo* OUTPUT_QUEUE_INFO,
    vulkanSwapchainSupportInfo* OUTPUT_SWAPCHAIN_SUPPORT) 
{
    // Evaluate device properties to determine if it meets the needs of our applcation.
    OUTPUT_QUEUE_INFO->graphicsFamilyIndex = -1;
    OUTPUT_QUEUE_INFO->presentFamilyIndex = -1;
    OUTPUT_QUEUE_INFO->computeFamilyIndex = -1;
    OUTPUT_QUEUE_INFO->transferFamilyIndex = -1;

    // Discrete GPU?
    if (REQUIREMENTS->discreteGpu) 
    {
        if (PROPERTIES->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
        {
            FORGE_LOG_INFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    unsigned int queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(DEVICE, &queueFamilyCount, 0);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(DEVICE, &queueFamilyCount, queue_families);

    // Look at each queue and see what queues it supports
    FORGE_LOG_INFO("Graphics | Present | Compute | Transfer | Name");
    unsigned char min_transfer_score = 255;
    for (unsigned int i = 0; i < queueFamilyCount; ++i) 
    {
        unsigned char currentTransferScore = 0;

        // Graphics queue?
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            OUTPUT_QUEUE_INFO->graphicsFamilyIndex = i;
            ++currentTransferScore;
        }

        // Compute queue?
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) 
        {
            OUTPUT_QUEUE_INFO->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        // Transfer queue?
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) 
        {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= min_transfer_score) 
            {
                min_transfer_score = currentTransferScore;
                OUTPUT_QUEUE_INFO->transferFamilyIndex = i;
            }
        }

        // Present queue?
        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(DEVICE, i, SURFACE, &supportsPresent));
        if (supportsPresent) 
        {
            OUTPUT_QUEUE_INFO->presentFamilyIndex = i;
        }
    }

    // Print out some info about the device
    FORGE_LOG_INFO("       %d |       %d |       %d |        %d | %s",
          OUTPUT_QUEUE_INFO->graphicsFamilyIndex != -1,
          OUTPUT_QUEUE_INFO->presentFamilyIndex != -1,
          OUTPUT_QUEUE_INFO->computeFamilyIndex != -1,
          OUTPUT_QUEUE_INFO->transferFamilyIndex != -1,
          PROPERTIES->deviceName);

    if (
        (!REQUIREMENTS->graphics || (REQUIREMENTS->graphics && OUTPUT_QUEUE_INFO->graphicsFamilyIndex != -1)) &&
        (!REQUIREMENTS->present || (REQUIREMENTS->present && OUTPUT_QUEUE_INFO->presentFamilyIndex != -1)) &&
        (!REQUIREMENTS->compute || (REQUIREMENTS->compute && OUTPUT_QUEUE_INFO->computeFamilyIndex != -1)) &&
        (!REQUIREMENTS->transfer || (REQUIREMENTS->transfer && OUTPUT_QUEUE_INFO->transferFamilyIndex != -1))) 
    {
        FORGE_LOG_INFO("Device meets queue requirements.");
        FORGE_LOG_TRACE("Graphics Family Index: %i", OUTPUT_QUEUE_INFO->graphicsFamilyIndex);
        FORGE_LOG_TRACE("Present Family Index:  %i", OUTPUT_QUEUE_INFO->presentFamilyIndex);
        FORGE_LOG_TRACE("Transfer Family Index: %i", OUTPUT_QUEUE_INFO->transferFamilyIndex);
        FORGE_LOG_TRACE("Compute Family Index:  %i", OUTPUT_QUEUE_INFO->computeFamilyIndex);

        // Query swapchain support.
        vulkanDeviceQuerySwapchainSupport(
            DEVICE,
            SURFACE,
            OUTPUT_SWAPCHAIN_SUPPORT);

        if (OUTPUT_SWAPCHAIN_SUPPORT->formatCount < 1 || OUTPUT_SWAPCHAIN_SUPPORT->presentModeCount < 1) 
        {
            if (OUTPUT_SWAPCHAIN_SUPPORT->formats) 
            {
                forgeFreeMemory(OUTPUT_SWAPCHAIN_SUPPORT->formats, sizeof(VkSurfaceFormatKHR) * OUTPUT_SWAPCHAIN_SUPPORT->formatCount, MEMORY_TAG_RENDERER);
            }
            if (OUTPUT_SWAPCHAIN_SUPPORT->presentModes) 
            {
                forgeFreeMemory(OUTPUT_SWAPCHAIN_SUPPORT->presentModes, sizeof(VkPresentModeKHR) * OUTPUT_SWAPCHAIN_SUPPORT->presentModeCount, MEMORY_TAG_RENDERER);
            }
            FORGE_LOG_INFO("Required swapchain support not present, skipping device.");
            return false;
        }

        // Device extensions.
        if (REQUIREMENTS->deviceExtensionNames) 
        {
            unsigned int availableExtensionCount = 0;
            VkExtensionProperties* availableExtensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                DEVICE,
                0,
                &availableExtensionCount,
                0));
            if (availableExtensionCount != 0) 
            {
                availableExtensions = forgeAllocateMemory(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    DEVICE,
                    0,
                    &availableExtensionCount,
                    availableExtensions));

                unsigned int requiredExtensionCount = listLength(REQUIREMENTS->deviceExtensionNames);
                for (unsigned int i = 0; i < requiredExtensionCount; ++i) 
                {
                    bool found = false;
                    for (unsigned int j = 0; j < availableExtensionCount; ++j) 
                    {
                        if (strcmp(REQUIREMENTS->deviceExtensionNames[i], availableExtensions[j].extensionName) == 0) 
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found) 
                    {
                        FORGE_LOG_INFO("Required extension not found: '%s', skipping device.", REQUIREMENTS->deviceExtensionNames[i]);
                        forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (REQUIREMENTS->samplerAnisotropy && !FEATURES->samplerAnisotropy) 
        {
            FORGE_LOG_INFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        // Device meets all requirements.
        return true;
    }

    return false;
}