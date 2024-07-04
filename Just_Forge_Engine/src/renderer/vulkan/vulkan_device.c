#include "vulkan_device.h"
#include "core/logger.h"
#include "core/memory.h"
#include <string.h>
#include "dataStructures/list.h"


// - - - | Necessary Structs | - - -


typedef struct gpuRequirements
{
    bool graphics; //draw calls
    bool present;  // present to screen
    bool compute;  // shaders
    bool transfer; // data transfer

    const char** gpuExtensionNames; //List of required extensions

    bool samplerAnisotropy; //Anisotropic filtering means that the texture is sampled at a different rate depending on the angle of the surface
    bool dedicated; // Integratde or dedicated
} gpuRequirements;

typedef struct gpuQueueInfo
{
    unsigned int graphicsFamilyIndex;
    unsigned int presentFamilyIndex;
    unsigned int computeFamilyIndex;
    unsigned int transferFamilyIndex;
} gpuQueueInfo;


// - - - | Functions | - - -


// - - - Forward Declarations - - -

bool selectGPU(vulkanContext* CONTEXT);

bool gpuMeetsRequirements(VkPhysicalDevice GPU, VkSurfaceKHR SURFACE, const VkPhysicalDeviceProperties* PROPERTIES, const VkPhysicalDeviceFeatures* FEATURES, const gpuRequirements* REQUIREMENTS, gpuQueueInfo* INFO, vulkanSwapchainSupportInfo* SWAPCHAIN_SUPPORT);

bool createVulkanDevice(vulkanContext* CONTEXT);


// - - - Implementations - - -

// - - - Create and destroy the Vulkan device
bool createVulkanDevice(vulkanContext* CONTEXT)
{
    if (!selectGPU(CONTEXT))
    {
        return false;
    }
    FORGE_LOG_INFO("Creating Logical Device");
    // NOTE: do not create additional queues for shared indices
    bool presentSharesGraphics = (CONTEXT->device.graphicsQueueIndex == CONTEXT->device.presentQueueIndex);
    bool computeSharesGraphics = (CONTEXT->device.graphicsQueueIndex == CONTEXT->device.computeQueueIndex);
    bool transferSharesGraphics = (CONTEXT->device.graphicsQueueIndex == CONTEXT->device.transferQueueIndex);
    unsigned int indexCount = 1;

    if (!presentSharesGraphics)
    {
        ++indexCount;
    }
    if (!computeSharesGraphics)
    {
        ++indexCount;
    }
    if (!transferSharesGraphics)
    {
        ++indexCount;
    }
    unsigned int indices[indexCount];
    unsigned char index = 0;
    indices[index++] = CONTEXT->device.graphicsQueueIndex;
    if (!presentSharesGraphics)
    {
        indices[index++] = CONTEXT->device.presentQueueIndex;
    }
    if (!computeSharesGraphics)
    {
        indices[index++] = CONTEXT->device.computeQueueIndex;
    }
    if (!transferSharesGraphics)
    {
        indices[index++] = CONTEXT->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[indexCount];
    for (unsigned int i = 0; i < indexCount; ++i)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;
        // TODO: enable this in future multi-threaded rendering
        /* if (indices[i] == CONTEXT->device.transferQueueIndex)
        {
            queueCreateInfos[i].queueCount = 2;
        }
        */
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        float queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    //Request device features
    // TODO: make this configurable
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE; // Requires a GPU that supports anisotropic filtering

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 1;
    const char* extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &extensions;

    //Create the device
    VK_CHECK(vkCreateDevice(CONTEXT->device.physicalDevice, &deviceCreateInfo, CONTEXT->allocator, &CONTEXT->device.logicalDevice));

    FORGE_LOG_INFO("Logical Device created");

    //Get queues
    vkGetDeviceQueue(CONTEXT->device.logicalDevice, CONTEXT->device.graphicsQueueIndex, 0, &CONTEXT->device.graphicsQueue);
    vkGetDeviceQueue(CONTEXT->device.logicalDevice, CONTEXT->device.presentQueueIndex, 0, &CONTEXT->device.presentQueue);
    vkGetDeviceQueue(CONTEXT->device.logicalDevice, CONTEXT->device.computeQueueIndex, 0, &CONTEXT->device.computeQueue);
    vkGetDeviceQueue(CONTEXT->device.logicalDevice, CONTEXT->device.transferQueueIndex, 0, &CONTEXT->device.transferQueue);

    FORGE_LOG_INFO("Queues retrieved");

    //Create a command pool for the graphics queue
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = CONTEXT->device.graphicsQueueIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(CONTEXT->device.logicalDevice, &commandPoolCreateInfo, CONTEXT->allocator, &CONTEXT->device.graphicsCommandPool));
    FORGE_LOG_INFO("Command Pool created for graphics queue");

    return true;
}

void destroyVulkanDevice(vulkanContext* CONTEXT)
{
    CONTEXT->device.graphicsQueue = 0;
    CONTEXT->device.presentQueue = 0;
    CONTEXT->device.computeQueue = 0;
    CONTEXT->device.transferQueue = 0;
    FORGE_LOG_INFO("Queues released");

    FORGE_LOG_INFO("Destroying command pool");
    vkDestroyCommandPool(CONTEXT->device.logicalDevice, CONTEXT->device.graphicsCommandPool, CONTEXT->allocator);

    FORGE_LOG_INFO("Destroying Logical Device");
    if (CONTEXT->device.logicalDevice)
    {
        vkDestroyDevice(CONTEXT->device.logicalDevice, CONTEXT->allocator);
        CONTEXT->device.logicalDevice = 0;
    }

    //Since physical devices are not created, they cant be destroyed
    FORGE_LOG_INFO("Releasing GPU resources");
    CONTEXT->device.physicalDevice = 0;

    if (CONTEXT->device.swapchainSupport.formats)
    {
        forgeFreeMemory(CONTEXT->device.swapchainSupport.formats, sizeof(VkSurfaceFormatKHR) * CONTEXT->device.swapchainSupport.formatCount, MEMORY_TAG_RENDERER);
        CONTEXT->device.swapchainSupport.formats = 0;
        CONTEXT->device.swapchainSupport.formatCount = 0;
    }

    if (CONTEXT->device.swapchainSupport.presentModes)
    {
        forgeFreeMemory(CONTEXT->device.swapchainSupport.presentModes, sizeof(VkPresentModeKHR) * CONTEXT->device.swapchainSupport.presentModeCount, MEMORY_TAG_RENDERER);
        CONTEXT->device.swapchainSupport.presentModes = 0;
        CONTEXT->device.swapchainSupport.presentModeCount = 0;
    }

    forgeZeroMemory(&CONTEXT->device.swapchainSupport, sizeof(CONTEXT->device.swapchainSupport.capabilities));

    CONTEXT->device.graphicsQueueIndex = -1;
    CONTEXT->device.presentQueueIndex = -1;
    CONTEXT->device.computeQueueIndex = -1;
    CONTEXT->device.transferQueueIndex = -1;

    forgeZeroMemory(&CONTEXT->device.properties, sizeof(CONTEXT->device.properties));
    FORGE_LOG_INFO("GPU resources released");
}

// - - -  Detect the depth format
bool vulkanDeviceDetectDepthFormat(vulkanDevice *DEVICE)
{
    //Format candidates
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

// - - - Query swapchain support
bool vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice GPU, VkSurfaceKHR SURFACE, vulkanSwapchainSupportInfo* INFO)
{
    //Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPU, SURFACE, &INFO->capabilities));

    //Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, SURFACE, &INFO->formatCount, 0));
    if (INFO->formatCount != 0)
    {
        if (!INFO->formats)
        {
            INFO->formats = forgeAllocateMemory(sizeof(VkSurfaceFormatKHR) * INFO->formatCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, SURFACE, &INFO->formatCount, INFO->formats));
    }

    //Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, SURFACE, &INFO->presentModeCount, 0));
    if (INFO->presentModeCount != 0)
    {
        if (!INFO->presentModes)
        {
            INFO->presentModes = forgeAllocateMemory(sizeof(VkPresentModeKHR) * INFO->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, SURFACE, &INFO->presentModeCount, INFO->presentModes));
    }
    return true;
}

// - - - Check if the GPU meets the requirements
bool gpuMeetsRequirements(
    VkPhysicalDevice GPU, 
    VkSurfaceKHR SURFACE, 
    const VkPhysicalDeviceProperties* PROPERTIES, 
    const VkPhysicalDeviceFeatures* FEATURES, 
    const gpuRequirements* REQUIREMENTS, 
    gpuQueueInfo* INFO, 
    vulkanSwapchainSupportInfo* SWAPCHAIN_SUPPORT)
{
    INFO->transferFamilyIndex = -1; //Negative on a unisnged number = insane number
    INFO->computeFamilyIndex = -1;
    INFO->presentFamilyIndex = -1;
    INFO->graphicsFamilyIndex = -1;

    if (REQUIREMENTS->dedicated && PROPERTIES->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        FORGE_LOG_DEBUG("Device is not a discrete GPU, skipping it.");
        return false;
    }

    unsigned int queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamilyCount, 0);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamilyCount, queueFamilies);

    //Look at each queue and see what queues it supports
    FORGE_LOG_INFO("Graphics    |   Present   |   Compute    |   Transfer");
    unsigned char minTransferScore = 255;
    for (unsigned int i = 0; i < queueFamilyCount; ++i)
    {
        unsigned char score = 0;

        //Graphics
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            INFO->graphicsFamilyIndex = i;
            ++score;
        }

        //Compute
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            INFO->computeFamilyIndex = i;
            ++score;
        }

        //Transfer
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            //Take the index if it is the current lowest. This increases the likelihood that it is a dedicated transfer queue
            if (score <= minTransferScore)
            {
                minTransferScore = score;
                INFO->transferFamilyIndex = i;
            }
        }

        //Present
        VkBool32 supportsPresentation = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(GPU, i, SURFACE, &supportsPresentation));
        if (supportsPresentation)
        {
            INFO->presentFamilyIndex = i;
        }
    }

        //print the info
        FORGE_LOG_INFO("      %d     |      %d      |      %d       |      %d      \n%s", (INFO->graphicsFamilyIndex != -1), (INFO->presentFamilyIndex != -1), (INFO->computeFamilyIndex != -1), (INFO->transferFamilyIndex != -1), PROPERTIES->deviceName);

        //If all the queues are found, return true
        if (
            (!REQUIREMENTS->graphics || (REQUIREMENTS->graphics && INFO->graphicsFamilyIndex != -1)) &&
            (!REQUIREMENTS->present || (REQUIREMENTS->present && INFO->presentFamilyIndex != -1)) &&
            (!REQUIREMENTS->compute || (REQUIREMENTS->compute && INFO->computeFamilyIndex != -1)) &&
            (!REQUIREMENTS->transfer || (REQUIREMENTS->transfer && INFO->transferFamilyIndex != -1))
           )
        {
            FORGE_LOG_DEBUG("%s meets requirements", PROPERTIES->deviceName);
            FORGE_LOG_TRACE("Graphics Family Index: %d, Present Family Index: %d, Compute Family Index: %d, Transfer Family Index: %d", INFO->graphicsFamilyIndex, INFO->presentFamilyIndex, INFO->computeFamilyIndex, INFO->transferFamilyIndex);
        
            //Query swapchain support
            vulkanDeviceQuerySwapchainSupport(GPU, SURFACE, SWAPCHAIN_SUPPORT);

            if (SWAPCHAIN_SUPPORT->formatCount < 1 || SWAPCHAIN_SUPPORT->presentModeCount < 1)
            {
                if (SWAPCHAIN_SUPPORT->formats)
                {
                    forgeFreeMemory(SWAPCHAIN_SUPPORT->formats, sizeof(VkSurfaceFormatKHR) * SWAPCHAIN_SUPPORT->formatCount, MEMORY_TAG_RENDERER);
                }

                if (SWAPCHAIN_SUPPORT->presentModes)
                {
                    forgeFreeMemory(SWAPCHAIN_SUPPORT->presentModes, sizeof(VkPresentModeKHR) * SWAPCHAIN_SUPPORT->presentModeCount, MEMORY_TAG_RENDERER);
                }

                FORGE_LOG_DEBUG("Required swapchain support not found, skipping device");
                return false;
            }

            //Device extensions
            if (REQUIREMENTS->gpuExtensionNames)
            {
                unsigned int extensionCount = 0;
                VkExtensionProperties* availableExtensions = 0;
                VK_CHECK(vkEnumerateDeviceExtensionProperties(GPU, 0, &extensionCount, 0));

                if (extensionCount != 0)
                {
                    availableExtensions = forgeAllocateMemory(sizeof(VkExtensionProperties) * extensionCount, MEMORY_TAG_RENDERER);
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(GPU, 0, &extensionCount, availableExtensions));

                unsigned int requiredExtensionCount = listLength(REQUIREMENTS->gpuExtensionNames);
                for (unsigned int i = 0; i < requiredExtensionCount; ++i)
                {
                    bool extensionFound = false;
                    for (unsigned int j = 0; j < extensionCount; ++j)
                    {
                        if (strcmp(REQUIREMENTS->gpuExtensionNames[i], availableExtensions[j].extensionName) == 0)
                        {
                            extensionFound = true;
                            break;
                        }
                    }

                    if (!extensionFound)
                    {
                        FORGE_LOG_DEBUG("Required extension not found: %s", REQUIREMENTS->gpuExtensionNames[i]);
                        forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, MEMORY_TAG_RENDERER);
        }

        //Sampler anisotropy
        if (REQUIREMENTS->samplerAnisotropy && !FEATURES->samplerAnisotropy)
        {
            FORGE_LOG_DEBUG("Device does not support sampler anisotropy, skipping it");
            return false;
        }

        return true;
    }
    return false;
}

// - - - Select the GPU
bool selectGPU(vulkanContext* CONTEXT)
{
    unsigned int gpuCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(CONTEXT->instance, &gpuCount, 0));
    if (gpuCount == 0)
    {
        FORGE_LOG_FATAL("No Graphics Cards or GPUs found");
        return false;
    }

    VkPhysicalDevice gpus[gpuCount];
    VK_CHECK(vkEnumeratePhysicalDevices(CONTEXT->instance, &gpuCount, gpus));
    for (unsigned int i = 0; i < gpuCount; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpus[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(gpus[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(gpus[i], &memory);

        // TODO: the requirements should be driven by game dev
        // Configuration
        gpuRequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.compute = true;
        requirements.dedicated = false; //Integrated GPUs are allowed
        requirements.samplerAnisotropy = true;
        requirements.gpuExtensionNames = listCreate(const char*);
        listAppend(requirements.gpuExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        gpuQueueInfo queueInfo = {};
        bool result = gpuMeetsRequirements(gpus[i], CONTEXT->surface, &properties, &features, &requirements, &queueInfo, &CONTEXT->device.swapchainSupport);

        if (result)
        {
            FORGE_LOG_INFO("GPU Selected: %s", properties.deviceName);
            switch (properties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    FORGE_LOG_INFO("Device Type: Integrated GPU");
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    FORGE_LOG_INFO("Device Type: Discrete GPU");
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    FORGE_LOG_INFO("Device Type: Virtual GPU");
                    break;

                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    FORGE_LOG_INFO("Device Type: CPU");
                    break;

                default:
                    FORGE_LOG_INFO("Device Type: Unknown");
                    break;
            }
            FORGE_LOG_INFO("GPU driver version: %d.%d.%d", VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));

            FORGE_LOG_INFO("API version: %d.%d.%d", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));

            for (unsigned int j = 0; j < memory.memoryHeapCount; ++j)
            {
                float memorySize = (((float)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f); //GB

                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    FORGE_LOG_DEBUG("GPU local memory: %.2f GB", memorySize);
                }
                else
                {
                    FORGE_LOG_DEBUG("GPU-CPU shared memory: %.2f GB", memorySize);
                }
            }

            CONTEXT->device.physicalDevice = gpus[i];
            CONTEXT->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            CONTEXT->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            CONTEXT->device.computeQueueIndex = queueInfo.computeFamilyIndex;
            CONTEXT->device.transferQueueIndex = queueInfo.transferFamilyIndex;

            CONTEXT->device.properties = properties;
            CONTEXT->device.features = features;
            CONTEXT->device.memory = memory;

            //YAY! We found a GPU that meets the requirements
            break;
        }
    }

    //Ensure that a device was selected
    if (!CONTEXT->device.physicalDevice)
    {
        FORGE_LOG_ERROR("No GPU was found that meets the requirements");
        return false;
    }

    return true;
}


