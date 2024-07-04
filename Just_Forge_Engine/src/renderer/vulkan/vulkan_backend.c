#include "vulkan_backend.h"

#include "vulkan_types.h"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_frame_buffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/memory.h"
#include "core/application.h"

#include "dataStructures/list.h"

#include "platform/platform.h"

#include "shaders/vulkan_object_shader.h"

#include <string.h>
#include <stdlib.h>
// - - - | Vulkan Setup | - - -


// - - - Vulkan Context
static vulkanContext context;
static unsigned int frameBufferWidth = 0;
static unsigned int frameBufferHeight = 0;

// - - - Debug Callback
VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MESSAGE_SEVERITY, VkDebugUtilsMessageTypeFlagsEXT MESSAGE_TYPES, const VkDebugUtilsMessengerCallbackDataEXT* CALLBACK_DATA, void* USER_DATA);

// - - - Command Buffer Creation
void createCommandBuffers(rendererBackend* BACKEND);

// - - - Generate framebuffers
void regenerateFramebuffers(rendererBackend* BACKEND, vulkanSwapchain* SWAPCHAIN, vulkanRenderpass* RENDERPASS);

// - - - FindMemoryIndex
int findMemoryIndex(unsigned int TYPE_FILTER, unsigned int PROPERTIES_FLAGS);

// - - - Recreate Swapchain
bool recreateSwapchain(rendererBackend* BACKEND);

// - - - Create Buffers
bool createBuffers(vulkanContext* CONTEXT);


// - - - | Vulkan as a Renderer Backend | - - -


bool vulkanRendererBackendInitialize(rendererBackend* BACKEND, const char* APPLICATION)
{
    //Function pointers
    context.findMemoryIndex = findMemoryIndex;

    //TODO: custom allocator
    context.allocator = 0;

    applicationGetFrameBufferSize(&frameBufferWidth, &frameBufferHeight);
    context.framebufferWidth = frameBufferWidth != 0 ? frameBufferWidth : 800;
    context.framebufferHeight = frameBufferHeight != 0 ? frameBufferHeight : 600;
    frameBufferWidth = 0;
    frameBufferHeight = 0;


    //Setup vulkan instance
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2; //Vulkan API version 1.2.0 seems popular
    appInfo.pApplicationName = APPLICATION; 
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //Version of the application
    appInfo.pEngineName = "Just Forge Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); //Version of the game engine

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    
    const char** requiredExtensions = listCreate(const char*);
    listAppend(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME); // Required for surface creation
    platformGetRequiredExtensions(&requiredExtensions);
    #if defined(_DEBUG)
        listAppend(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Required for debug utils
        FORGE_LOG_DEBUG("Required extensions: ");
        for (unsigned long long i = 0; i < listLength(requiredExtensions); ++i)
        {
            FORGE_LOG_DEBUG("    %s", requiredExtensions[i]);
        }
    #endif

    createInfo.enabledExtensionCount = listLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;


    //Validation layers
    const char** requiredValidationLayerNames = 0;
    unsigned int requiredValidationLayerCount = 0;

    #if defined(_DEBUG)
        FORGE_LOG_DEBUG("Validation layers enabled, Counting...");

        //The list of required validation layers
        requiredValidationLayerNames = listCreate(const char*);
        listAppend(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
        requiredValidationLayerCount = listLength(requiredValidationLayerNames);

        //Get the available validation layers
        unsigned int availableLayerCount = 0;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
        VkLayerProperties* availableLayers = listReserve(VkLayerProperties, availableLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

        //Verify that the required validation layers are available
        for (unsigned int i = 0; i < requiredValidationLayerCount; ++i)
        {
            FORGE_LOG_DEBUG("Checking for validation layer: %s", requiredValidationLayerNames[i]);
            bool layerFound = false;
            for (unsigned int j = 0; j < availableLayerCount; ++j)
            {
                if (strcmp(requiredValidationLayerNames[i], availableLayers[j].layerName) == 0)
                {
                    layerFound = true;
                    FORGE_LOG_DEBUG("Validation layer found: %s", requiredValidationLayerNames[i]);
                    break;
                }
            }

            if (!layerFound)
            {
                FORGE_LOG_FATAL("Validation layer not found: %s", requiredValidationLayerNames[i]);
                return false;
            }
        }
        FORGE_LOG_DEBUG("All required validation layers found.");
    #endif

    createInfo.enabledLayerCount = requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    FORGE_LOG_INFO("Vulkan instance created");


    //Setup debug messenger
    #if defined(_DEBUG)
        FORGE_LOG_DEBUG("Creating debug messenger...");
        unsigned int logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debugCreateInfo.messageSeverity = logSeverity;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = vkDebugCallback;
        debugCreateInfo.pUserData = 0;

        PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
        FORGE_ASSERT_MESSAGE(func, "Failed to create debug messenger!");
        VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
        FORGE_LOG_DEBUG("Vulkan debugger created.");
    #endif
    
    //Setup vulkan surface
    FORGE_LOG_INFO("Creating vulkan surface");
    if (!platformCreateSurface(&context))
    {
        FORGE_LOG_ERROR("Failed to create vulkan surface");
        return false;
    }

    //Setup vulkan device
    if (!createVulkanDevice(&context))
    {
        FORGE_LOG_ERROR("Failed to create vulkan device");
        return false;
    }

    // Swapchain
    createVulkanSwapchain(&context, context.framebufferWidth, context.framebufferHeight, &context.swapchain);

    // Renderpass
    createRenderpass(&context, &context.mainRenderpass, 0, 0, context.framebufferWidth, context.framebufferHeight, (float[4]){(float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, 1.0f}, 1.0f, 0);

    // Framebuffers
    context.swapchain.framebuffers = listReserve(vulkanFramebuffer, context.swapchain.imageCount);
    regenerateFramebuffers(BACKEND, &context.swapchain, &context.mainRenderpass);

    // Command buffer
    createCommandBuffers(BACKEND);

    // Sync
    context.imageAvailableSemaphores = listReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.renderFinishedSemaphores = listReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = listReserve(vulkanFence, context.swapchain.maxFramesInFlight);

    for (unsigned char i = 0; i < context.swapchain.maxFramesInFlight; ++i)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(context.device.logicalDevice, &semaphoreInfo, context.allocator, &context.imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(context.device.logicalDevice, &semaphoreInfo, context.allocator, &context.renderFinishedSemaphores[i]));

        //Create the fence signalled, indicating that the first frame has already been rendered
        //This prevents the application to wait for the first frame to be rendered becuase no frame is rendered before a frame before it has been rendered.
        createFence(&context, true, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers because the initial state should be 0, and will be 0 when not used. Actual fences are owned by this list
    context.imagesInFlight = listReserve(vulkanFence, context.swapchain.imageCount);
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        context.imagesInFlight[i] = 0;
    }

    // Object shader
    if (!createObjectShader(&context, &context.objectShader))
    {
        FORGE_LOG_ERROR("Failed to initialize object shader");
        return false;
    }

    // Buffers
    if (!createBuffers(&context))
    {
        FORGE_LOG_ERROR("Failed to create buffers");
        return false;
    }
    
    FORGE_LOG_INFO("Vulkan Renderer Initialized");
    return true;
}

void vulkanRendererBackendShutdown(rendererBackend* BACKEND)
{
    //Destroy in the opposite order of creation
    vkDeviceWaitIdle(context.device.logicalDevice);
    for (unsigned char i = 0; i < context.swapchain.maxFramesInFlight; ++i)
    {
        if (context.imageAvailableSemaphores[i])
        {
            vkDestroySemaphore(context.device.logicalDevice, context.imageAvailableSemaphores[i], context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if (context.renderFinishedSemaphores[i])
        {
            vkDestroySemaphore(context.device.logicalDevice, context.renderFinishedSemaphores[i], context.allocator);
            context.renderFinishedSemaphores[i] = 0;
        }
        destroyFence(&context, &context.inFlightFences[i]);
    }
    listDestroy(context.imageAvailableSemaphores);
    listDestroy(context.renderFinishedSemaphores);
    listDestroy(context.inFlightFences);
    listDestroy(context.imagesInFlight);
    context.imageAvailableSemaphores = 0;
    context.renderFinishedSemaphores = 0;
    context.inFlightFences = 0;
    context.imagesInFlight = 0;
    
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        if (context.graphicsCommandBuffers[i].handle)
        {
            commandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }
    listDestroy(context.graphicsCommandBuffers);

    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        destroyFrameBuffer(&context, &context.swapchain.framebuffers[i]);
    }

    FORGE_LOG_INFO("Destroying vulkan renderpass");
    destroyRenderpass(&context, &context.mainRenderpass);  

    FORGE_LOG_INFO("Destroying vulkan swapchain");
    destroyVulkanSwapchain(&context, &context.swapchain);

    FORGE_LOG_INFO("Destroying vulkan device");
    destroyVulkanDevice(&context);

    FORGE_LOG_INFO("Destroying vulkan surface");
    if (context.surface)
    {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }
        
    FORGE_LOG_INFO("Destroying vulkan debugger");
    if (context.debugMessenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }

    FORGE_LOG_INFO("Destroying Vulkan Instance");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkanRendererBackendResized(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT)
{
    // update the frambuffer size generation, a counter which indicates when the framebuffer size is updated
    frameBufferWidth = WIDTH;
    frameBufferHeight = HEIGHT;
    context.framebufferSizeGeneration++;
    FORGE_LOG_DEBUG("Framebuffer resized to %d x %d with generation %d", WIDTH, HEIGHT, context.framebufferSizeGeneration);
}

bool vulkanRendererBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME)
{
    vulkanDevice* gpu = &context.device;
    
    //Check if recreating swapchain and boot out
    if (context.recreateSwapchain)
    {
        VkResult result = vkDeviceWaitIdle(gpu->logicalDevice);
        if (resultSuccess(result))
        {
            FORGE_LOG_ERROR("vulkan_render_backend : vulkanRenderBackendBeginFrame : Failed to wait for device idle(1) : '%s'", vulkanResultToString(result, true));
            return false;
        }
        FORGE_LOG_DEBUG("Recreating swapchain... booting");
        return false;
    }

    //Check if framebuffer has been resized. If so, a recreation of the swapchain is needed
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration)
    {
        VkResult result = vkDeviceWaitIdle(gpu->logicalDevice);
        if (!resultSuccess(result))
        {
            FORGE_LOG_ERROR("vulkan_render_backend : vulkanRenderBackendBeginFrame : Failed to wait for device idle(2) : '%s'", vulkanResultToString(result, true));
            return false;
        }
        // If the swapchain recreation failed (because, for example, the window was minimized).
        // Boot out before unsetting the flag
        if (!recreateSwapchain(BACKEND))
        {
            return false;
        }

        FORGE_LOG_DEBUG("Framebuffer resized, booting...");
        return false;
    }
    
    //Wait for the execution of the current frame to be complete. The fence being free will allow this one to move on
    if (!waitForFence(&context, &context.inFlightFences[context.currentFrame], UINT64_MAX))
    {
        FORGE_LOG_WARNING("vulkan_render_backend : vulkanRenderBackendBeginFrame : Failed to wait for in flight fence");
        return false;
    }
    
    //Acquire the next image
    if (!vulkanSwapchainAquireNextImageIndex(&context, &context.swapchain, UINT64_MAX, context.imageAvailableSemaphores[context.currentFrame], 0, &context.imageIndex))
    {
        FORGE_LOG_WARNING("vulkan_render_backend : vulkanRenderBackendBeginFrame : Failed to acquire next image");
        return false;
    }

    vulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    commandBufferReset(commandBuffer);
    commandBufferBegin(commandBuffer, false, false, false);

    //Dynamic state (consistent with OpenGL)
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)context.framebufferHeight;
    viewport.width = (float)context.framebufferWidth;
    viewport.height = -(float)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;

    //Begin renderpass
    beginRenderpass(commandBuffer, &context.mainRenderpass, context.swapchain.framebuffers[context.imageIndex].handle);

    return true;
}

bool vulkanRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME)
{
    vulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];

    //End renderpass
    endRenderpass(commandBuffer, &context.mainRenderpass);

    //End command buffer
    commandBufferEnd(commandBuffer);

    // Make sure the previous frame is not using this image (i.e., its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE)
    {
        waitForFence(&context, context.imagesInFlight[context.imageIndex], UINT64_MAX);
    }

    // Mark the image fence as now being in use by this frame
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    //Reset the fence for use in the next frame
    resetFence(&context, &context.inFlightFences[context.currentFrame]);

    //Submit the queue and wait for the operation to complete
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    //Semaphores to be signalled when the queue is complete
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.renderFinishedSemaphores[context.currentFrame];

    //Semaphores to wait on before the queue is executed
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    //Each semaphoree waits on the corresponding pipeline stages to complete 1:1 ratio
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT is the stage where the color attachment is written to. It prevents subsequent color attachment
    // Writes from executing until the semaphore signals (i.e., one frame is submitted at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(context.device.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.currentFrame].handle);

    commandBufferUpdateSubmits(commandBuffer);
    //End of queue submission

    //Present the image
    vulkanSwapchainPresentImage(&context, &context.swapchain, context.device.graphicsQueue, context.device.presentQueue, context.renderFinishedSemaphores[context.currentFrame], context.imageIndex);

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MESSAGE_SEVERITY, VkDebugUtilsMessageTypeFlagsEXT MESSAGE_TYPES, const VkDebugUtilsMessengerCallbackDataEXT* CALLBACK_DATA, void* USER_DATA)
{
    switch (MESSAGE_SEVERITY)
    {
        default:

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            FORGE_LOG_ERROR(CALLBACK_DATA->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            FORGE_LOG_WARNING(CALLBACK_DATA->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            FORGE_LOG_DEBUG(CALLBACK_DATA->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            FORGE_LOG_TRACE(CALLBACK_DATA->pMessage);
            break;
    }
    return VK_FALSE;
}

int findMemoryIndex(unsigned int TYPE_FILTER, unsigned int PROPERTY_FLAGS)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);

    for (unsigned int i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((TYPE_FILTER & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & PROPERTY_FLAGS) == PROPERTY_FLAGS)
        {
            return i;
        }
    }

    FORGE_LOG_WARNING("Failed to find suitable memory type");
    return -1;
}

void createCommandBuffers(rendererBackend* BACKEND)
{
    if (!context.graphicsCommandBuffers)
    {
        context.graphicsCommandBuffers = listReserve(vulkanCommandBuffer, context.swapchain.imageCount);
        for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
        {
            forgeZeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkanCommandBuffer));
        }
    }

    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        if (context.graphicsCommandBuffers[i].handle)
        {
            commandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
        }
        forgeZeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkanCommandBuffer));
        commandBufferAllocate(&context, context.device.graphicsCommandPool, true, &context.graphicsCommandBuffers[i]);
    }

    FORGE_LOG_INFO("Command buffers created");
}

void regenerateFramebuffers(rendererBackend* BACKEND, vulkanSwapchain* SWAPCHAIN, vulkanRenderpass* RENDERPASS)
{
    for (unsigned int i = 0; i < SWAPCHAIN->imageCount; ++i)
    {
        // TODO: make this dynamic based on config of attachments
        unsigned int attachmentCount = 2;
        VkImageView attachments[2] = {SWAPCHAIN->imageViews[i], SWAPCHAIN->depthAttachment.view};

        createFrameBuffer(&context, RENDERPASS, context.framebufferWidth, context.framebufferHeight, attachmentCount, attachments, &context.swapchain.framebuffers[i]);
    }
}

bool recreateSwapchain(rendererBackend* BACKEND)
{
    //If already recreating swapchain, boot out
    if (context.recreateSwapchain)
    {
        FORGE_LOG_DEBUG("vulkan_render_backend : recreateSwapchain : Already recreating swapchain");
        return false;
    }

    //Detect if the window is too smol to be rendered
    if (context.framebufferWidth == 0 || context.framebufferHeight == 0)
    {
        FORGE_LOG_DEBUG("vulkan_render_backend : recreateSwapchain : Framebuffer too small to render");
        return false;
    }

    //Mark as recreating swapchain
    context.recreateSwapchain = true;

    //Wait for any operations to complete
    vkDeviceWaitIdle(context.device.logicalDevice);

    //Clear out
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        context.imagesInFlight[i] = 0;
    }

    //Requery support
    vulkanDeviceQuerySwapchainSupport(context.device.physicalDevice, context.surface, &context.device.swapchainSupport);
    vulkanDeviceDetectDepthFormat(&context.device);

    recreateVulkanSwapchain(&context, frameBufferWidth, frameBufferHeight, &context.swapchain);

    //Sync the frambuffer size 
    context.framebufferWidth = frameBufferWidth;
    context.framebufferHeight = frameBufferHeight;
    context.mainRenderpass.width = frameBufferWidth;
    context.mainRenderpass.height = frameBufferHeight;
    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;
    for (int i = 0; i < 3; ++i)
    {
        context.mainRenderpass.clearColor[i] += rand() % 10 / 10.0f;
        if (context.mainRenderpass.clearColor[i] > 1.0f)
        {
            context.mainRenderpass.clearColor[i] = 0.0f;
        }
    }
    frameBufferWidth = 0;
    frameBufferHeight = 0;

    //Updat frambuffer size generation
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    //cleanup swapchain
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        commandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    }

    //Recreate framebuffers
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i)
    {
        destroyFrameBuffer(&context, &context.swapchain.framebuffers[i]);
    }

    context.mainRenderpass.x = 0;
    context.mainRenderpass.y = 0;
    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;

    regenerateFramebuffers(BACKEND, &context.swapchain, &context.mainRenderpass);

    createCommandBuffers(BACKEND);

    //Cleaeet the recreating flag
    context.recreateSwapchain = false;

    return true;
}

bool createBuffers(vulkanContext* CONTEXT)
{

}
