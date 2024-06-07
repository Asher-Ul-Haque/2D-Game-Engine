#pragma once
#include "defines.h"
#include "core/asserts.h"
#include <vulkan/vulkan.h>


// - - - Assert on Vulkan check
#define VK_CHECK(EXPRESSION) \
    { \
        FORGE_ASSERT(EXPRESSION == VK_SUCCESS); \
    }


// - - - | Structs | - - -


// - - - Vulkan Swapchain Support Info
typedef struct vulkanSwapchainSupportInfo
{
    VkSurfaceCapabilitiesKHR capabilities;
    unsigned int formatCount;
    VkSurfaceFormatKHR* formats;
    unsigned int presentModeCount;
    VkPresentModeKHR* presentModes;
} vulkanSwapchainSupportInfo; 

// - - - Vulkan Command buffer - - -

// - - - Vulkan Command buffer States
typedef enum vulkanCommandBufferState
{
    COMMAND_BUFFER_STATE_READY, // Command buffer is ready to be recorded
    COMMAND_BUFFER_STATE_RECORDING, // Command buffer is currently being recorded
    COMMAND_BUFFER_STATE_IN_RENDERPASS, // Command buffer is currently in a renderpass
    COMMAND_BUFFER_STATE_FINISHED, // Command buffer has been recorded and is ready to be submitted
    COMMAND_BUFFER_STATE_SUBMITTED, // Command buffer has been submitted to the GPU
    COMMAND_BUFFER_STATE_NONE // Command buffer is not in a valid state
} vulkanCommandBufferState; 

// - - - Vulkan Command buffer
typedef struct vulkanCommandBuffer
{
    VkCommandBuffer handle;
    vulkanCommandBufferState state;
} vulkanCommandBuffer;

// - - - Vulkan Device
typedef struct vulkanDevice
{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    vulkanSwapchainSupportInfo swapchainSupport;

    int graphicsQueueIndex;
    int presentQueueIndex;
    int computeQueueIndex;
    int transferQueueIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
} vulkanDevice;

// - - - Vulkan Image
typedef struct vulkanImage
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    unsigned int width;
    unsigned int height;
} vulkanImage;


// - - - Vulkan Renderpass - - - 

// - - - Vulkan Renderpass States
typedef enum vulkanRenderpassState
{
    RENDERPASS_STATE_READY, // Renderpass is ready to be recorded
    RENDERPASS_STATE_RECORDING, // Renderpass is currently being recorded
    RENDERPASS_STATE_FINISHED, // Renderpass has been recorded and is ready to be submitted
    RENDERPASS_STATE_SUBMITTED, // Renderpass has been submitted to the GPU
    RENDERPASS_STATE_NONE // Renderpass is not in a valid state
} vulkanRenderpassState;

// - - - Vulkan Renderpass
typedef struct vulkanRenderpass
{
    VkRenderPass handle;
    float x, y, width, height;
    float clearColor[4];
    float depth;
    unsigned int pencil;
    vulkanRenderpassState state;
} vulkanRenderpass;

// - - - Vulkan Swapchain
typedef struct vulkanSwapchain 
{
    VkSurfaceFormatKHR imageFormat; // Vulkan requires us to know the format of the images in the swapchain
    unsigned char maxFramesInFlight; // Maximum number of images in the swapchain
    VkSwapchainKHR handle;
    VkImage* images;
    unsigned int imageCount;
    VkImageView* imageViews; // Image views are required to use the images in the swapchain. Image views describe how to access an image and which part of the image to access
    vulkanImage depthAttachment;
} vulkanSwapchain;

// - - - Vulkan Context
typedef struct vulkanContext
{
    unsigned int framebufferHeight;
    unsigned int framebufferWidth;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    vulkanDevice device;
    vulkanSwapchain swapchain;
    vulkanRenderpass mainRenderpass;
    unsigned int imageIndex;
    unsigned int currentFrame;
    bool8 recreateSwapchain;
    int (*findMemoryIndex)(unsigned int TYPE_FILTER, unsigned int PROPERTY_FLAGS);
    #if defined(_DEBUG)
        VkDebugUtilsMessengerEXT debugMessenger;
    #endif
} vulkanContext;
