#include "vulkan_backend.h"

#include "vulkan_types.h"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"
#include "vulkan_buffer.h"

#include "core/logger.h"
#include <string.h>
#include "core/memory.h"
#include "core/application.h"

#include "dataStructures/list.h"

#include "math/math_types.h"

#include "platform/platform.h"

// Shaders
#include "shaders/vulkan_object_shader.h"

// static Vulkan context
static vulkanContext context;
static unsigned int cachedFramebufferWidth = 0;
static unsigned int cachedFramebufferHeight = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

int findMemoryIndex(unsigned int TYPE_FILTER, unsigned int PROPERTY_FLAGS);
bool createBuffers(vulkanContext* CONTEXT);

void createCommandBuffers(rendererBackend* BACKEND);
void regenerateFramebuffers(rendererBackend* BACKEND, vulkanSwapchain* SWAPCHAIN, vulkanRenderpass* RENDERPASS);
bool recreateSwapchain(rendererBackend* BACKEND);

void uploadDataRange(vulkanContext* CONTEXT, VkCommandPool POOL, VkFence FENCE, VkQueue QUEUE, vulkanBuffer* BUFFER, unsigned long long OFFSET, unsigned long long SIZE, void* DATA) 
{
    // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkanBuffer staging;
    vulkanBufferCreate(CONTEXT, SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    // Load the data into the staging buffer.
    vulkanBufferLoadData(CONTEXT, &staging, 0, SIZE, 0, DATA);

    // Perform the copy from staging to the device local buffer.
    vulkanBufferCopyTo(CONTEXT, POOL, FENCE, QUEUE, staging.handle, 0, BUFFER->handle, OFFSET, SIZE);

    // Clean up the staging buffer.
    vulkanBufferDestroy(CONTEXT, &staging);
}

bool vulkanRendererBackendInitialize(rendererBackend* BACKEND, const char* APP_NAME) 
{
    // Function pointers
    context.findMemoryIndex = findMemoryIndex;

    // TODO: custom allocator.
    context.allocator = 0;

    applicationGetFrameBufferSize(&cachedFramebufferWidth, &cachedFramebufferHeight);
    context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Setup Vulkan instance.
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Just Forge Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    // Obtain a list of required extensions
    const char** requiredExtensions = listCreate(const char*);
    listAppend(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);  // Generic surface extension
    platformGetRequiredExtensionNames(&requiredExtensions);       // Platform-specific extension(s)
#if defined(_DEBUG)
    listAppend(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // debug utilities

    FORGE_LOG_DEBUG("Required extensions:");
    unsigned int length = listLength(requiredExtensions);
    for (unsigned int i = 0; i < length; ++i) 
    {
        FORGE_LOG_DEBUG(requiredExtensions[i]);
    }
#endif

    createInfo.enabledExtensionCount = listLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    // Validation layers.
    const char** requiredValidationLayerNames = 0;
    unsigned int requiredValidationLayerCount = 0;

// If validation should be done, get a list of the required validation layert names
// and make sure they exist. Validation layers should only be enabled on non-release builds.
#if defined(_DEBUG)
    FORGE_LOG_INFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    requiredValidationLayerNames = listCreate(const char*);
    listAppend(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
    requiredValidationLayerCount = listLength(requiredValidationLayerNames);

    // Obtain a list of available validation layers
    unsigned int availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
    VkLayerProperties* availableLayers = listReserve(VkLayerProperties, availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    // Verify all required layers are available.
    for (unsigned int i = 0; i < requiredValidationLayerCount; ++i) 
    {
        FORGE_LOG_INFO("Searching for layer: %s...", requiredValidationLayerNames[i]);
        bool found = false;
        for (unsigned int j = 0; j < availableLayerCount; ++j) 
        {
            if (strcmp(requiredValidationLayerNames[i], availableLayers[j].layerName) == 0) 
            {
                found = true;
                FORGE_LOG_INFO("Found.");
                break;
            }
        }

        if (!found) 
        {
            FORGE_LOG_FATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
            return false;
        }
    }
    FORGE_LOG_INFO("All required validation layers are present.");
#endif

    createInfo.enabledLayerCount = requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    FORGE_LOG_INFO("Vulkan Instance created.");

    // Debugger
#if defined(_DEBUG)
    FORGE_LOG_DEBUG("Creating Vulkan debugger...");
    unsigned int logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vk_debug_callback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    FORGE_ASSERT_MESSAGE(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
    FORGE_LOG_DEBUG("Vulkan debugger created.");
#endif

    // Surface
    FORGE_LOG_DEBUG("Creating Vulkan surface...");
    if (!platformCreateVulkanSurface(&context)) 
    {
        FORGE_LOG_ERROR("Failed to create platform surface!");
        return false;
    }
    FORGE_LOG_DEBUG("Vulkan surface created.");

    // Device creation
    if (!vulkanDeviceCreate(&context)) 
    {
        FORGE_LOG_ERROR("Failed to create device!");
        return false;
    }

    // Swapchain
    vulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain);

    vulkanRenderpassCreate(
        &context,
        &context.mainRenderpass,
        0, 0, context.framebufferWidth, context.framebufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0);

    // Swapchain framebuffers.
    context.swapchain.framebuffers = listReserve(vulkanFramebuffer, context.swapchain.imageCount);
    regenerateFramebuffers(BACKEND, &context.swapchain, &context.mainRenderpass);

    // Create command buffers.
    createCommandBuffers(BACKEND);

    // Create sync objects.
    context.imageAvailableSemaphores = listReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = listReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = listReserve(vulkanFence, context.swapchain.maxFramesInFlight);

    for (unsigned char i = 0; i < context.swapchain.maxFramesInFlight; ++i) 
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo, context.allocator, &context.queueCompleteSemaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        vulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
    // by this list.
    context.imagesInFlight = listReserve(vulkanFence, context.swapchain.imageCount);
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        context.imagesInFlight[i] = 0;
    }

    // Create builtin shaders
    if (!vulkanObjectShaderCreate(&context, &context.objectShader)) 
    {
        FORGE_LOG_ERROR("Error loading built-in basic_lighting shader.");
        return false;
    }

    createBuffers(&context);

    // TODO: temporary test code
    const unsigned int vertCount = 4;
    Vertex3D verts[vertCount];
    forgeZeroMemory(verts, sizeof(Vertex3D) * vertCount);

    verts[0].position.x = 0.0;
    verts[0].position.y = -0.5;

    verts[1].position.x = 0.5;
    verts[1].position.y = 0.5;

    verts[2].position.x = 0;
    verts[2].position.y = 0.5;

    verts[3].position.x = 0.5;
    verts[3].position.y = -0.5;

    const unsigned int indexCount = 6;
    unsigned int indices[indexCount] = {0, 1, 2, 0, 3, 1};

    uploadDataRange(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.objectVertexBuffer, 0, sizeof(Vertex3D) * vertCount, verts);
    uploadDataRange(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.objectIndexBuffer, 0, sizeof(unsigned int) * indexCount, indices);
    // TODO: end temp code

    FORGE_LOG_INFO("Vulkan renderer initialized successfully.");
    return true;
}

void vulkanRendererBackendShutdown(rendererBackend* BACKEND) 
{
    vkDeviceWaitIdle(context.device.logicalDevice);

    // Destroy in the opposite order of creation.
    // Destroy buffers
    vulkanBufferDestroy(&context, &context.objectVertexBuffer);
    vulkanBufferDestroy(&context, &context.objectIndexBuffer);

    vulkanObjectShaderDestroy(&context, &context.objectShader);

    // Sync objects
    for (unsigned char i = 0; i < context.swapchain.maxFramesInFlight; ++i) 
    {
        if (context.imageAvailableSemaphores[i]) 
        {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if (context.queueCompleteSemaphores[i]) 
        {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }
        vulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }
    listDestroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    listDestroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    listDestroy(context.inFlightFences);
    context.inFlightFences = 0;

    listDestroy(context.imagesInFlight);
    context.imagesInFlight = 0;

    // Command buffers
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        if (context.graphicsCommandBUffers[i].handle) 
        {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBUffers[i]);
            context.graphicsCommandBUffers[i].handle = 0;
        }
    }
    listDestroy(context.graphicsCommandBUffers);
    context.graphicsCommandBUffers = 0;

    // Destroy framebuffers
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    // Renderpass
    vulkanRenderpassDestroy(&context, &context.mainRenderpass);

    // Swapchain
    vulkanSwapchainDestroy(&context, &context.swapchain);

    FORGE_LOG_DEBUG("Destroying Vulkan device...");
    vulkanDeviceDestroy(&context);

    FORGE_LOG_DEBUG("Destroying Vulkan surface...");
    if (context.surface) 
    {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

#if defined(_DEBUG)
    FORGE_LOG_DEBUG("Destroying Vulkan debugger...");
    if (context.debugMessenger) 
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }
#endif

    FORGE_LOG_DEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkanRendererBackendOnResized(rendererBackend* BACKEND, unsigned short WIDTH, unsigned short HEIGHT) 
{
    // Update the "framebuffer size generation", a counter which indicates when the
    // framebuffer size has been updated.
    cachedFramebufferWidth = WIDTH;
    cachedFramebufferHeight = HEIGHT;
    context.framebufferSizeGeneration++;

    FORGE_LOG_INFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", WIDTH, HEIGHT, context.framebufferSizeGeneration);
}

bool vulkanRendererBackendBeginFrame(rendererBackend* BACKEND, float DELTA_TIME) 
{
    vulkanDevice* device = &context.device;

    // Check if recreating swap chain and boot out.
    if (context.recreatingSwapchain) 
    {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!vulkanResultIsSuccess(result)) 
        {
            FORGE_LOG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", vulkanResultToString(result, true));
            return false;
        }
        FORGE_LOG_INFO("Recreating swapchain, booting.");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration) 
    {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!vulkanResultIsSuccess(result)) 
        {
            FORGE_LOG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: '%s'", vulkanResultToString(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // boot out before unsetting the flag.
        if (!recreateSwapchain(BACKEND)) 
        {
            return false;
        }

        FORGE_LOG_INFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
    if (!vulkanFenceWait(
            &context,
            &context.inFlightFences[context.currentFrame],
            UINT64_MAX)) 
    {
        FORGE_LOG_WARNING("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
    // This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if (!vulkanSwapchainAcquireNextImageIndex(
            &context,
            &context.swapchain,
            UINT64_MAX,
            context.imageAvailableSemaphores[context.currentFrame],
            0,
            &context.imageIndex)) 
    {
        return false;
    }

    // Begin recording commands.
    vulkanCommandBuffer* commandBuffer = &context.graphicsCommandBUffers[context.imageIndex];
    vulkanCommandBufferReset(commandBuffer);
    vulkanCommandBufferBegin(commandBuffer, false, false, false);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)context.framebufferHeight;
    viewport.width = (float)context.framebufferWidth;
    viewport.height = -(float)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    // Begin the render pass.
    vulkanRenderpassBegin(
        commandBuffer,
        &context.mainRenderpass,
        context.swapchain.framebuffers[context.imageIndex].handle);

    // TODO: temporary test code
    vulkanObjectShaderUse(&context, &context.objectShader);

    // Bind vertex buffer at offset.
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &context.objectVertexBuffer.handle, (VkDeviceSize*)offsets);

    // Bind index buffer at offset.
    vkCmdBindIndexBuffer(commandBuffer->handle, context.objectIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

    // Issue the draw.
    vkCmdDrawIndexed(commandBuffer->handle, 6, 1, 0, 0, 0);
    // TODO: end temporary test code

    return true;
}

bool vulkanRendererBackendEndFrame(rendererBackend* BACKEND, float DELTA_TIME) 
{
    vulkanCommandBuffer* commandBuffer = &context.graphicsCommandBUffers[context.imageIndex];

    // End renderpass
    vulkanRenderpassEnd(commandBuffer, &context.mainRenderpass);

    vulkanCommandBufferEnd(commandBuffer);

    // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) 
    {  // was frame
        vulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            UINT64_MAX);
    }

    // Mark the image fence as in-use by this frame.
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    // Reset the fence for use on the next frame
    vulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    // Submit the queue and wait for the operation to complete.
    // Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    // The semaphore(s) to be signaled when the queue is complete.
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    // Wait semaphore ensures that the operation cannot begin until the image is available.
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue,
        1,
        &submitInfo,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS) 
    {
        FORGE_LOG_ERROR("vkQueueSubmit failed with result: %s", vulkanResultToString(result, true));
        return false;
    }

    vulkanCommandBufferUpdateSubmitted(commandBuffer);
    // End queue submission

    // Give the image back to the swapchain.
    vulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) 
    {
    switch (message_severity) 
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            FORGE_LOG_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            FORGE_LOG_WARNING(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            FORGE_LOG_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            FORGE_LOG_TRACE(callback_data->pMessage);
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
        // Check each memory type to see if its bit is set to 1.
        if (TYPE_FILTER & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & PROPERTY_FLAGS) == PROPERTY_FLAGS) 
        {
            return i;
        }
    }

    FORGE_LOG_WARNING("Unable to find suitable memory type!");
    return -1;
}

void createCommandBuffers(rendererBackend* BACKEND) 
{
    if (!context.graphicsCommandBUffers) 
    {
        context.graphicsCommandBUffers = listReserve(vulkanCommandBuffer, context.swapchain.imageCount);
        for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
        {
            forgeZeroMemory(&context.graphicsCommandBUffers[i], sizeof(vulkanCommandBuffer));
        }
    }

    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        if (context.graphicsCommandBUffers[i].handle) 
        {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBUffers[i]);
        }
        forgeZeroMemory(&context.graphicsCommandBUffers[i], sizeof(vulkanCommandBuffer));
        vulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            true,
            &context.graphicsCommandBUffers[i]);
    }

    FORGE_LOG_DEBUG("Vulkan command buffers created.");
}

void regenerateFramebuffers(rendererBackend* BACKEND, vulkanSwapchain* SWAPCHAIN, vulkanRenderpass* RENDERPASS) 
{
    for (unsigned int i = 0; i < SWAPCHAIN->imageCount; ++i) 
    {
        // TODO: make this dynamic based on the currently configured attachments
        unsigned int attachmentCount = 2;
        VkImageView attachments[] = {
            SWAPCHAIN->views[i],
            SWAPCHAIN->depthAttachment.view};

        vulkanFramebufferCreate(
            &context,
            RENDERPASS,
            context.framebufferWidth,
            context.framebufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.framebuffers[i]);
    }
}

bool recreateSwapchain(rendererBackend* BACKEND) 
{
    // If already being recreated, do not try again.
    if (context.recreatingSwapchain) 
    {
        FORGE_LOG_DEBUG("recreate_swapchain called when already recreating. Booting.");
        return false;
    }

    // Detect if the window is too small to be drawn to
    if (context.framebufferWidth == 0 || context.framebufferHeight == 0) 
    {
        FORGE_LOG_DEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    // Mark as recreating if the dimensions are valid.
    context.recreatingSwapchain = true;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logicalDevice);

    // Clear these out just in case.
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        context.imagesInFlight[i] = 0;
    }

    // Requery support
    vulkanDeviceQuerySwapchainSupport(
        context.device.physicalDevice,
        context.surface,
        &context.device.swapchainSupport);
    vulkanDeviceDetectDepthFormat(&context.device);

    vulkanSwapchainRecreate(
        &context,
        cachedFramebufferWidth,
        cachedFramebufferHeight,
        &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebufferWidth = cachedFramebufferWidth;
    context.framebufferHeight = cachedFramebufferHeight;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Update framebuffer size generation.
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    // cleanup swapchain
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        vulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBUffers[i]);
    }

    // Framebuffers.
    for (unsigned int i = 0; i < context.swapchain.imageCount; ++i) 
    {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    context.mainRenderpass.x = 0;
    context.mainRenderpass.y = 0;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    regenerateFramebuffers(BACKEND, &context.swapchain, &context.mainRenderpass);

    createCommandBuffers(BACKEND);

    // Clear the recreating flag.
    context.recreatingSwapchain = false;

    return true;
}

bool createBuffers(vulkanContext* CONTEXT) 
{
    VkMemoryPropertyFlagBits memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const unsigned long long vertex_buffer_size = sizeof(Vertex3D) * 1024 * 1024;
    if (!vulkanBufferCreate(
            CONTEXT,
            vertex_buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memoryPropertyFlags,
            true,
            &CONTEXT->objectVertexBuffer)) 
    {
        FORGE_LOG_ERROR("Error creating vertex buffer.");
        return false;
    }
    CONTEXT->geometryVertexOffset = 0;

    const unsigned long long indexBufferSize = sizeof(unsigned int) * 1024 * 1024;
    if (!vulkanBufferCreate(
            CONTEXT,
            indexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memoryPropertyFlags,
            true,
            &CONTEXT->objectIndexBuffer)) 
    {
        FORGE_LOG_ERROR("Error creating vertex buffer.");
        return false;
    }
    CONTEXT->geometryIndexOffset = 0;

    return true;
}