#include "vulkan_object_shader.h"

#include "core/logger.h"
#include "core/memory.h"
#include "math/math_types.h"

#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_buffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


bool vulkanObjectShaderCreate(vulkanContext* CONTEXT, vulkanObjectShader* OUTPUT_SHADER) 
{
    // Shader module init per stage.
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        if (!createShaderModule(CONTEXT, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i], stageTypes[i], i, OUTPUT_SHADER->stages)) 
        {
            FORGE_LOG_ERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // Global Descriptors
    VkDescriptorSetLayoutBinding globalUBOLayoutBinding;
    globalUBOLayoutBinding.binding = 0;
    globalUBOLayoutBinding.descriptorCount = 1;
    globalUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalUBOLayoutBinding.pImmutableSamplers = 0;
    globalUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    globalLayoutInfo.bindingCount = 1;
    globalLayoutInfo.pBindings = &globalUBOLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(
        CONTEXT->device.logicalDevice, 
        &globalLayoutInfo, 
        CONTEXT->allocator, 
        &OUTPUT_SHADER->globalDescriptorSetLayout));

    VkDescriptorPoolSize globalPoolSize;
    globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalPoolSize.descriptorCount = CONTEXT->swapchain.imageCount;

    VkDescriptorPoolCreateInfo globalPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    globalPoolInfo.poolSizeCount = 1;
    globalPoolInfo.pPoolSizes = &globalPoolSize;
    globalPoolInfo.maxSets = CONTEXT->swapchain.imageCount;
    VK_CHECK(vkCreateDescriptorPool(
        CONTEXT->device.logicalDevice, 
        &globalPoolInfo, 
        CONTEXT->allocator, 
        &OUTPUT_SHADER->globalDescriptorPool));

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)CONTEXT->framebufferHeight;
    viewport.width = (float)CONTEXT->framebufferWidth;
    viewport.height = -(float)CONTEXT->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = CONTEXT->framebufferWidth;
    scissor.extent.height = CONTEXT->framebufferHeight;

    // Attributes
    unsigned int offset = 0;
    const int attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
    // Position
    VkFormat formats[attribute_count] = 
    {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    unsigned long long sizes[attribute_count] = 
    {
        sizeof(Vector3D)
    };
    for (unsigned int i = 0; i < attribute_count; ++i) 
    {
        attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
        attribute_descriptions[i].location = i;  // attrib location
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    // Descriptor Set Layouts
    const int desciptorSetLayoutCount = 1;
    VkDescriptorSetLayout layouts[desciptorSetLayoutCount] = {OUTPUT_SHADER->globalDescriptorSetLayout};

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stageCreateInfos[OBJECT_SHADER_STAGE_COUNT];
    forgeZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        stageCreateInfos[i].sType = OUTPUT_SHADER->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = OUTPUT_SHADER->stages[i].shaderStageCreateInfo;
    }

    if (!vulkanGraphicsPipelineCreate(
            CONTEXT,
            &CONTEXT->mainRenderpass,
            attribute_count,
            attribute_descriptions,
            desciptorSetLayoutCount,
            layouts,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfos,
            viewport,
            scissor,
            false,
            &OUTPUT_SHADER->pipeline)) 
    {
        FORGE_LOG_ERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    // Global Uniform buffer

    if (!vulkanBufferCreate(
        CONTEXT,
        sizeof(globalUBO) * 3,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        true,
        &OUTPUT_SHADER->globalUniformBuffer))
    {
        FORGE_LOG_ERROR("Vulkan Buffer creation failed for builtin object shader");
        return false;
    }

    // Allocate global descriptor sets
    VkDescriptorSetLayout globalLayouts[3] = {
        OUTPUT_SHADER->globalDescriptorSetLayout,
        OUTPUT_SHADER->globalDescriptorSetLayout,
        OUTPUT_SHADER->globalDescriptorSetLayout};

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = OUTPUT_SHADER->globalDescriptorPool;
    allocInfo.descriptorSetCount = 3;
    allocInfo.pSetLayouts = globalLayouts;
    VK_CHECK(vkAllocateDescriptorSets(
        CONTEXT->device.logicalDevice, 
        &allocInfo, 
        OUTPUT_SHADER->globalDescriptorSet));

    return true;
}

void vulkanObjectShaderDestroy(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER) 
{
    VkDevice logicalDevice = CONTEXT->device.logicalDevice;

    // Destroy the uniform buffers
    vulkanBufferDestroy(CONTEXT, &SHADER->globalUniformBuffer);
    
    // Destory Pipeline
    vulkanPipelineDestroy(CONTEXT, &SHADER->pipeline);

    // Destory Global Descriptor Pool
    vkDestroyDescriptorPool(CONTEXT->device.logicalDevice, SHADER->globalDescriptorPool, CONTEXT->allocator);

    // Destroy Descriptor Set Layouts
    vkDestroyDescriptorSetLayout(CONTEXT->device.logicalDevice, SHADER->globalDescriptorSetLayout, CONTEXT->allocator);

    // Destroy shader modules.
    for (unsigned int i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) 
    {
        vkDestroyShaderModule(CONTEXT->device.logicalDevice, SHADER->stages[i].handle, CONTEXT->allocator);
        SHADER->stages[i].handle = 0;
    }
}

void vulkanObjectShaderUse(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER) 
{
    unsigned int imageIndex = CONTEXT->imageIndex;
    vulkanPipelineBind(&CONTEXT->graphicsCommandBUffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &SHADER->pipeline);
}

void vulkanObjectShaderUpdateGlobalState(vulkanContext* CONTEXT, struct vulkanObjectShader* SHADER)
{
    unsigned int imageIndex = CONTEXT->imageIndex;
    VkCommandBuffer commandBuffer = CONTEXT->graphicsCommandBUffers[imageIndex].handle;
    VkDescriptorSet globalDescriptor = SHADER->globalDescriptorSet[imageIndex];

   // if (!SHADER->descriptorUpdated[imageIndex]) 
    //{
        // Bind the global descriptor set to be updated
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, SHADER->pipeline.pipelineLayout, 0, 1, &globalDescriptor, 0, 0);

        // Configure the descriptor for the given index
        unsigned int range = sizeof(globalUBO);
        unsigned long long offset = 0;

        // Copy the data to buffer
        vulkanBufferLoadData(CONTEXT, 
                             &SHADER->globalUniformBuffer, offset, 
                             range, 
                             0, 
                             &SHADER->globalUniformBufferObject);

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = SHADER->globalUniformBuffer.handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        // Update descriptor sets
        VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet = globalDescriptor;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(CONTEXT->device.logicalDevice, 1, &descriptorWrite, 0, 0);
       // SHADER->descriptorUpdated[imageIndex] = true;
    //}

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, SHADER->pipeline.pipelineLayout, 0, 1, &globalDescriptor, 0, 0);
}
