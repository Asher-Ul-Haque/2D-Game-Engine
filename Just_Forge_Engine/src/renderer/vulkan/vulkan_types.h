#pragma once

#include "defines.h"
#include "core/asserts.h"

#include <vulkan/vulkan.h>

// Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr)               \
    {                                \
        FORGE_ASSERT(expr == VK_SUCCESS); \
    }



// - - - All fucking types needed for rendering - - -

typedef struct vulkanBuffer 
{
    unsigned long long totalSize;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    bool isLocked;
    VkDeviceMemory memory;
    int memoryIndex;
    unsigned int memoryPropertyFlags;
} vulkanBuffer;

typedef struct vulkanSwapchainSupportInfo 
{
    VkSurfaceCapabilitiesKHR capabilities;
    unsigned int formatCount;
    VkSurfaceFormatKHR* formats;
    unsigned int presentModeCount;
    VkPresentModeKHR* presentModes;
} vulkanSwapchainSupportInfo;

typedef struct vulkanDevice 
{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    vulkanSwapchainSupportInfo swapchainSupport;
    int graphicsQueueIndex;
    int presentQueueIndex;
    int transferQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkCommandPool graphicsCommandPool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
} vulkanDevice;

typedef struct vulkanImage 
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    unsigned int width;
    unsigned int height;
} vulkanImage;

typedef enum vulkanRenderPassState 
{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkanRenderPassState;

typedef struct vulkanRenderpass 
{
    VkRenderPass handle;
    float x, y, w, h;
    float r, g, b, a;

    float depth;
    unsigned int stencil;

    vulkanRenderPassState state;
} vulkanRenderpass;

typedef struct vulkanFramebuffer 
{
    VkFramebuffer handle;
    unsigned int attachment_count;
    VkImageView* attachments;
    vulkanRenderpass* renderpass;
} vulkanFramebuffer;

typedef struct vulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    unsigned char maxFramesInFlight;
    VkSwapchainKHR handle;
    unsigned int imageCount;
    VkImage* images;
    VkImageView* views;

    vulkanImage depthAttachment;

    // framebuffers used for on-screen rendering.
    vulkanFramebuffer* framebuffers;
} vulkanSwapchain;

typedef enum vulkanCommandBufferState 
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkanCommandBufferState;

typedef struct vulkanCommandBuffer 
{
    VkCommandBuffer handle;

    // Command buffer state.
    vulkanCommandBufferState state;
} vulkanCommandBuffer;

typedef struct vulkanFence 
{
    VkFence handle;
    bool isSignaled;
} vulkanFence;

typedef struct vulkanShaderStage 
{
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
} vulkanShaderStage;

typedef struct vulkanPipeline 
{
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
} vulkanPipeline;

#define OBJECT_SHADER_STAGE_COUNT 2
typedef struct vulkanObjectShader 
{
    // vertex, fragment
    vulkanShaderStage stages[OBJECT_SHADER_STAGE_COUNT];
    vulkanPipeline pipeline;
} vulkanObjectShader;

typedef struct vulkanContext 
{

    // The framebuffer's current width.
    unsigned int framebufferWidth;

    // The framebuffer's current height.
    unsigned int framebufferHeight;

    // Current generation of framebuffer size. If it does not match framebuffer_size_last_generation,
    // a new one should be generated.
    unsigned long long framebufferSizeGeneration;

    // The generation of the framebuffer when it was last created. Set to framebuffer_size_generation
    // when updated.
    unsigned long long framebufferSizeLastGeneration;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    vulkanDevice device;

    vulkanSwapchain swapchain;
    vulkanRenderpass mainRenderpass;

    vulkanBuffer objectVertexBuffer;
    vulkanBuffer objectIndexBuffer;

    // list
    vulkanCommandBuffer* graphicsCommandBUffers;

    // list
    VkSemaphore* imageAvailableSemaphores;

    // list
    VkSemaphore* queueCompleteSemaphores;

    unsigned int inFlightFenceCount;
    vulkanFence* inFlightFences;

    // Holds pointers to fences which exist and are owned elsewhere.
    vulkanFence** imagesInFlight;

    unsigned int imageIndex;
    unsigned int currentFrame;

    bool recreatingSwapchain;

    vulkanObjectShader objectShader;

    unsigned long long geometryVertexOffset;
    unsigned long long geometryIndexOffset;

    int (*findMemoryIndex)(unsigned int TYPE_FILTER, unsigned int PROPERTY_FLAGS);

} vulkanContext;