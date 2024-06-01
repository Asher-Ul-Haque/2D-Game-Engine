#include "vulkan_device.h"
#include "core/logger.h"
#include "core/memory.h"
#include <string.h>
#include "dataStructures/list.h"


// - - - | Necessary Structs | - - -


typedef struct gpuRequirements
{
    bool8 graphics; //draw calls
    bool8 present;  // present to screen
    bool8 compute;  // shaders
    bool8 transfer; // data transfer

    const char** gpuExtensionNames; //List of required extensions

    bool8 samplerAnisotropy; //Anisotropic filtering means that the texture is sampled at a different rate depending on the angle of the surface
    bool8 dedicated; // Integratde or dedicated
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

bool8 selectGPU(vulkanContext* CONTEXT);

bool8 gpuMeetsRequirements(VkPhysicalDevice GPU, VkSurfaceKHR SURFACE, const VkPhysicalDeviceProperties* PROPERTIES, const VkPhysicalDeviceFeatures* FEATURES, const gpuRequirements* REQUIREMENTS, gpuQueueInfo* INFO, vulkanSwapchainSupportInfo* SWAPCHAIN_SUPPORT);

bool8 createVulkanDevice(vulkanContext* CONTEXT);


// - - - Implementations - - -

// - - - Create and destroy the Vulkan device
bool8 createVulkanDevice(vulkanContext* CONTEXT)
{
    if (!selectGPU(CONTEXT))
    {
        return FALSE;
    }
    return TRUE;
}

void destroyVulkanDevice(vulkanContext* CONTEXT)
{
}

// - - - Query swapchain support
bool8 vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice GPU, VkSurfaceKHR SURFACE, vulkanSwapchainSupportInfo* INFO)
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
    return TRUE;
}

// - - - Check if the GPU meets the requirements
bool8 gpuMeetsRequirements(
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
        FORGE_LOG_INFO("Device is not a discrete GPU, skipping it.");
        return FALSE;
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
        FORGE_LOG_INFO("    %d |    %d |    %d |    %d | %s", (INFO->graphicsFamilyIndex != -1), (INFO->presentFamilyIndex != -1), (INFO->computeFamilyIndex != -1), (INFO->transferFamilyIndex != -1), PROPERTIES->deviceName);

        //If all the queues are found, return true
        if (
            (!REQUIREMENTS->graphics || (REQUIREMENTS->graphics && INFO->graphicsFamilyIndex != -1)) &&
            (!REQUIREMENTS->present || (REQUIREMENTS->present && INFO->presentFamilyIndex != -1)) &&
            (!REQUIREMENTS->compute || (REQUIREMENTS->compute && INFO->computeFamilyIndex != -1)) &&
            (!REQUIREMENTS->transfer || (REQUIREMENTS->transfer && INFO->transferFamilyIndex != -1))
           )
        {
            FORGE_LOG_INFO("%s meets requirements", PROPERTIES->deviceName);
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

                FORGE_LOG_INFO("Required swapchain support not found, skipping device");
                return FALSE;
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
                    bool8 extensionFound = FALSE;
                    for (unsigned int j = 0; j < extensionCount; ++j)
                    {
                        if (strcmp(REQUIREMENTS->gpuExtensionNames[i], availableExtensions[j].extensionName) == 0)
                        {
                            extensionFound = TRUE;
                            break;
                        }
                    }

                    if (!extensionFound)
                    {
                        FORGE_LOG_INFO("Required extension not found: %s", REQUIREMENTS->gpuExtensionNames[i]);
                        forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }
            }
            forgeFreeMemory(availableExtensions, sizeof(VkExtensionProperties) * extensionCount, MEMORY_TAG_RENDERER);
        }

        //Sampler anisotropy
        if (REQUIREMENTS->samplerAnisotropy && !FEATURES->samplerAnisotropy)
        {
            FORGE_LOG_INFO("Device does not support sampler anisotropy, skipping it");
            return FALSE;
        }

        return TRUE;
    }
    return FALSE;
}

// - - - Select the GPU
bool8 selectGPU(vulkanContext* CONTEXT)
{
    unsigned int gpuCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(CONTEXT->instance, &gpuCount, 0));
    if (gpuCount == 0)
    {
        FORGE_LOG_FATAL("No Graphics Cards or GPUs found");
        return FALSE;
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
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        requirements.compute = TRUE;
        requirements.dedicated = FALSE; //Integrated GPUs are allowed
        requirements.samplerAnisotropy = TRUE;
        requirements.gpuExtensionNames = listCreate(const char*);
        listAppend(requirements.gpuExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        gpuQueueInfo queueInfo = {};
        bool8 result = gpuMeetsRequirements(gpus[i], CONTEXT->surface, &properties, &features, &requirements, &queueInfo, &CONTEXT->device.swapchainSupport);

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
                    FORGE_LOG_INFO("GPU local memory: %.2f GB", memorySize);
                }
                else
                {
                    FORGE_LOG_INFO("GPU-CPU shared memory: %.2f GB", memorySize);
                }
            }

            CONTEXT->device.physicalDevice = gpus[i];
            CONTEXT->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            CONTEXT->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            CONTEXT->device.computeQueueIndex = queueInfo.computeFamilyIndex;

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
        return FALSE;
    }

    FORGE_LOG_INFO("GPU selected");
    return TRUE;
}

