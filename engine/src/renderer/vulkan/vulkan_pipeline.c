#include "vulkan_pipeline.h"
#include "vulkan_utils.h"

#include "core/memory.h"
#include "core/logger.h"

#include "math/math_types.h"

bool vulkanGraphicsPipelineCreate(
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
    vulkanPipeline* OUTPUT_PIPELINE) 
{
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
    //Viewport state 
    VkPipelineViewportStateCreateInfo viewportState= {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
=======
    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c
    viewportState.viewportCount = 1;
    viewportState.pViewports = &VIEWPORT;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &SCISSOR;

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
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
=======
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = IS_WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    // Multisampling.
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingCreateInfo.minSampleShading = 1.0f;
    multisamplingCreateInfo.pSampleMask = 0;
    multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing.
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c
    VkPipelineDepthStencilStateCreateInfo depthStencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
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
=======
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
    forgeZeroMemory(&colorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorblendStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorblendStageCreateInfo.logicOpEnable = VK_FALSE;
    colorblendStageCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorblendStageCreateInfo.attachmentCount = 1;
    colorblendStageCreateInfo.pAttachments = &colorBlendAttachmentState;
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c

    // Dynamic state
    const unsigned int dynamicCount = 3;
    VkDynamicState dynamicStates[dynamicCount] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateCreateInfo.dynamicStateCount = dynamicCount;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates;

    // Vertex input
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;  // Binding index
    bindingDescription.stride = sizeof(Vertex3D);
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after each vertex
    
    //Attributes
=======
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // Move to next data entry for each vertex.

    // Attributes
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = ATTRIBUTE_COUNT;
    vertexInputInfo.pVertexAttributeDescriptions = ATTRIBUTES;

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
    //Input assembly
=======
    // Input assembly
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
    //Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    //Descriptor set layouts
    pipelineLayoutInfo.setLayoutCount = DESCRIPTOR_SET_LAYOUT_COUNT;
    pipelineLayoutInfo.pSetLayouts = DESCRIPTOR_SET_LAYOUTS;
=======
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    // Descriptor set layouts
    pipelineLayoutCreateInfo.setLayoutCount = DESCRIPTOR_SET_LAYOUT_COUNT;
    pipelineLayoutCreateInfo.pSetLayouts = DESCRIPTOR_SET_LAYOUTS;
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c

    // Create the pipeline layout.
    VK_CHECK(vkCreatePipelineLayout(
<<<<<<< HEAD:engine/src/renderer/vulkan/vulkan_pipeline.c
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
=======
        CONTEXT->device.logicalDevice,
        &pipelineLayoutCreateInfo,
        CONTEXT->allocator,
        &OUTPUT_PIPELINE->pipelineLayout));

    // Pipeline create
    VkGraphicsPipelineCreateInfo pipeline_create_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeline_create_info.stageCount = STAGE_COUNT;
    pipeline_create_info.pStages = STAGES;
    pipeline_create_info.pVertexInputState = &vertexInputInfo;
    pipeline_create_info.pInputAssemblyState = &inputAssembly;

    pipeline_create_info.pViewportState = &viewportState;
    pipeline_create_info.pRasterizationState = &rasterizerCreateInfo;
    pipeline_create_info.pMultisampleState = &multisamplingCreateInfo;
    pipeline_create_info.pDepthStencilState = &depthStencil;
    pipeline_create_info.pColorBlendState = &colorblendStageCreateInfo;
    pipeline_create_info.pDynamicState = &dynamicStateCreateInfo;
    pipeline_create_info.pTessellationState = 0;

    pipeline_create_info.layout = OUTPUT_PIPELINE->pipelineLayout;

    pipeline_create_info.renderPass = RENDERPASS->handle;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;
>>>>>>> bug-Fix-3D:Just_Forge_Engine/src/renderer/vulkan/vulkan_pipeline.c

    VkResult result = vkCreateGraphicsPipelines(
        CONTEXT->device.logicalDevice,
        VK_NULL_HANDLE,
        1,
        &pipeline_create_info,
        CONTEXT->allocator,
        &OUTPUT_PIPELINE->handle);

    if (vulkanResultIsSuccess(result)) 
    {
        FORGE_LOG_DEBUG("Graphics pipeline created!");
        return true;
    }

    FORGE_LOG_ERROR("vkCreateGraphicsPipelines failed with %s.", vulkanResultToString(result, true));
    return false;
}

void vulkanPipelineDestroy(vulkanContext* CONTEXT, vulkanPipeline* PIPELINE) 
{
    if (PIPELINE) 
    {
        // Destroy pipeline
        if (PIPELINE->handle) 
        {
            vkDestroyPipeline(CONTEXT->device.logicalDevice, PIPELINE->handle, CONTEXT->allocator);
            PIPELINE->handle = 0;
        }

        // Destroy layout
        if (PIPELINE->pipelineLayout) 
        {
            vkDestroyPipelineLayout(CONTEXT->device.logicalDevice, PIPELINE->pipelineLayout, CONTEXT->allocator);
            PIPELINE->pipelineLayout = 0;
        }
    }
}

void vulkanPipelineBind(vulkanCommandBuffer* COMMAND_BUFFER, VkPipelineBindPoint BIND_POINT, vulkanPipeline* PIPELINE) 
{
    vkCmdBindPipeline(COMMAND_BUFFER->handle, BIND_POINT, PIPELINE->handle);
}