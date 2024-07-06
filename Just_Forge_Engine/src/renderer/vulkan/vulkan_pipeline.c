#include "vulkan_pipeline.h"
#include "vulkan_utils.h"

#include "core/memory.h"
#include "core/logger.h"

#include "math/math_types.h"

bool createGraphicsPipeline(
    vulkanContext* CONTEXT,
    vulkanRenderpass* RENDERPASS,
    unsigned int ATTRIBUTE_COUNT,
    VkVertexInputAttributeDescription* ATTRIBUTES,
    unsigned int DESCRIPTOR_SET_LAYOUT_COUNT,
    VkDescriptorSetLayout* DESCRIPTOR_SET_LAYOUTS,
    unsigned int STAGE_COUNT,
    VkPipelineShaderStageCreateInfo* STAGES,
    VkViewport VIEWPORT,
    VkRect2D SCISSOR,
    bool IS_WIREFRAME,
    vulkanPipeline* PIPELINE)
{
    //Viewport state 
    VkPipelineViewportStateCreateInfo viewportState= {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &VIEWPORT;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &SCISSOR;

    //Rasterization static_assert(, "Oopsie");
    VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = IS_WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    //Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = 0;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    //Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    //color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    forgeZeroMemory(&colorBlendAttachment, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    //Dynamic state 
    const unsigned int dynamicStateCount = 3;
    VkDynamicState dynamicStates[dynamicStateCount] = {
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR, 
        VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = dynamicStateCount;
    dynamicState.pDynamicStates = dynamicStates;

    //Vertex input
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex3D);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after each vertex
    
    //Attributes
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = ATTRIBUTE_COUNT;
    vertexInputInfo.pVertexAttributeDescriptions = ATTRIBUTES;

    //Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    //Descriptor set layouts
    pipelineLayoutInfo.setLayoutCount = DESCRIPTOR_SET_LAYOUT_COUNT;
    pipelineLayoutInfo.pSetLayouts = DESCRIPTOR_SET_LAYOUTS;

    //Create the pipeline layout
    VK_CHECK(vkCreatePipelineLayout(
             CONTEXT->device.logicalDevice,
             &pipelineLayoutInfo,
             CONTEXT->allocator,
             &PIPELINE->layout));

    //Pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = STAGE_COUNT;
    pipelineInfo.pStages = STAGES;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;

    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pTessellationState = 0;

    pipelineInfo.layout = PIPELINE->layout;

    pipelineInfo.renderPass = RENDERPASS->handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(
        CONTEXT->device.logicalDevice, 
        VK_NULL_HANDLE, 
        1, 
        &pipelineInfo, 
        CONTEXT->allocator, 
        &PIPELINE->handle);

    if (resultSuccess(result))
    {
        FORGE_LOG_INFO("Successfully created graphics pipeline");
        return true;
    }

    FORGE_LOG_ERROR("Failed to create graphics pipeline");
    return false;
}

void destroyGraphicsPipeline(vulkanContext* CONTEXT, vulkanPipeline* PIPELINE)
{
    if (PIPELINE)
    {
        //Destroy the pipeline
        if (PIPELINE->handle)
        {
            vkDestroyPipeline(CONTEXT->device.logicalDevice, PIPELINE->handle, CONTEXT->allocator);
            PIPELINE->handle = 0;
        }

        if (PIPELINE->layout)
        {
            vkDestroyPipelineLayout(CONTEXT->device.logicalDevice, PIPELINE->layout, CONTEXT->allocator);
            PIPELINE->layout = 0;
        }
    }
    else
    {
        FORGE_LOG_WARNING("Attempted to destroy an invalid pipeline");
    }
}

void bindGraphicsPipeline(vulkanCommandBuffer* COMMAND_BUFFER, VkPipelineBindPoint BIND_POINT, vulkanPipeline* PIPELINE)
{
    vkCmdBindPipeline(COMMAND_BUFFER->handle, BIND_POINT, PIPELINE->handle);
}
