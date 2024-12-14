#include "renderer/pipeline_builder.h"

PipelineBuilder::PipelineBuilder(const Device& device) : _device(device) {
	clear();
}

Pipeline PipelineBuilder::buildPipeline() {
    Logger* logger = Logger::get_logger();

    if (pipelineLayout == VK_NULL_HANDLE) {
        throw std::runtime_error("No pipeline layout fouund! Please include a pipeline layout to the pipeline builder!");
    }

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Setup dummy color blending. Not using transparent objects yet. 
    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };
    
    // Not used yet so just initialize it to default
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    // Dynamic states allow us to specify these things at command recording instead of pipeline creation
    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = &state[0]
    };

    // Build the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = nullptr // Using dynamic rendering
    };

    VkPipeline vkPipeline;
    if (vkCreateGraphicsPipelines(_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline");
    }

    Pipeline newPipeline(&_device, vkPipeline, pipelineLayout);
    logger->print("Successfully Created Render Pipeline");

    return newPipeline;
}

void PipelineBuilder::clear() {
    vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    shaderStages.clear();
    inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    colorBlendAttachment = {};
    multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    renderingInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    colorAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipelineLayout = VK_NULL_HANDLE;
}

void PipelineBuilder::setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader) {
    shaderStages.clear(); shaderStages.reserve(2);
    shaderStages.push_back(Shader::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
    shaderStages.push_back(Shader::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void PipelineBuilder::setInputTopology(VkPrimitiveTopology topology) {
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // Not using for now
}

void PipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.0f; // Setting this to a default of 1.0
}

void PipelineBuilder::setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
}

void PipelineBuilder::setMultisampling() {
    // TODO: Defaulting to none until I learn more about this
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // no multisampling: 1 sample per pixel
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::disableBlending() {
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::setColorAttachmentFormat(VkFormat format) {
    colorAttachmentFormat = format;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
}

void PipelineBuilder::setDepthAttachmentFormat(VkFormat format) {
    renderingInfo.depthAttachmentFormat = format;
}

void PipelineBuilder::setDepthTest(VkCompareOp compareOp) {
    depthStencil.depthTestEnable = compareOp == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    depthStencil.depthWriteEnable = compareOp == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    depthStencil.depthCompareOp = compareOp;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
}

void PipelineBuilder::setPipelineLayout(VkPipelineLayout layout) {
    pipelineLayout = layout;
}

void PipelineBuilder::setPipelineLayout(VkPipelineLayoutCreateInfo layoutInfo) {
    VkPipelineLayout layout = createPipelineLayout(_device, layoutInfo);
    pipelineLayout = layout;
}

void PipelineBuilder::setVertexInputState(VkPipelineVertexInputStateCreateInfo createInfo) {
    vertexInputInfo = createInfo;
}

//-------------------------- Static methods ---------------------------------//

VkPipelineLayout PipelineBuilder::createPipelineLayout(const Device& device, VkPipelineLayoutCreateInfo createInfo) {
    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device.device(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    return pipelineLayout;
}

VkPipelineLayoutCreateInfo PipelineBuilder::pipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    return createInfo;
}

VkPipelineVertexInputStateCreateInfo PipelineBuilder::vertexInputStateCreateInfo() {
    VkPipelineVertexInputStateCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0
    };
    return createInfo;
}