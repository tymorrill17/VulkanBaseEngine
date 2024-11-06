#include "renderer/engine.h"

static std::vector<PoolSizeRatio> renderDescriptorSetSizes = {
	{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
	{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
	{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
};

static std::vector<PoolSizeRatio> computeDescriptorSetSizes = {
	{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
};

static uint32_t NUM_PARTICLES = 1;

static Pipeline buildDefaultPipeline(const Device& device, PipelineBuilder& pipelineBuilder, const Swapchain& swapchain) {

	pipelineBuilder.clear();

	std::string baseDir = static_cast<std::string>(BASE_DIR);
	std::string folderDir = baseDir + "\\shaders\\";

	VkShaderModule defaultVertShader;
	Shader::loadShaderModule(folderDir + "circle.vert.spv", device, defaultVertShader);
	VkShaderModule defaultFragShader;
	Shader::loadShaderModule(folderDir + "circle.frag.spv", device, defaultFragShader);
	pipelineBuilder.setShaders(defaultVertShader, defaultFragShader);

	VkPipelineLayout defaultLayout = PipelineBuilder::createPipelineLayout(device, PipelineBuilder::pipelineLayoutCreateInfo());
	pipelineBuilder.setPipelineLayout(defaultLayout);
	pipelineBuilder.setVertexInputState(PipelineBuilder::vertexInputStateCreateInfo());

	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.setMultisampling();
	pipelineBuilder.disableBlending();
	pipelineBuilder.setDepthTest();
	pipelineBuilder.setColorAttachmentFormat(swapchain.imageFormat());

	return pipelineBuilder.buildPipeline();
}

Engine::Engine(Window& window) : 
	window(window), 
	instance("EngineTest", "VulkanEngineV2", true),
	debugMessenger(instance),
	device(instance, window, Instance::deviceExtensions),
	allocator(device, instance),
	swapchain(device, window),
	pipelineBuilder(device),
	frameNumber(0),
	drawImage(device, allocator, VkExtent3D{ window.extent().width, window.extent().height, 1 }, swapchain.imageFormat(),
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, VkMemoryAllocateFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), VK_IMAGE_ASPECT_COLOR_BIT),
	defaultPipeline(device),
	descriptorLayoutBuilder(device),
	renderDescriptorSets(device, 10, renderDescriptorSetSizes),
	computeDescriptorSets(device, 10, computeDescriptorSetSizes) {

	logger = Logger::get_logger();

	frames.reserve(swapchain.framesInFlight());
	for (int i = 0; i < frames.capacity(); i++) {
		frames.emplace_back(std::move(device));
	}

	defaultPipeline = buildDefaultPipeline(device, pipelineBuilder, swapchain);

	// Set up descriptor sets here
	// Particles will be sent to the GPU using a storage buffer
	// Camera data will be sent using a uniform buffer (or push constants... need to refresh my knowledge of them)

	logger->print("Engine Initiated!");
}

static VkRenderingInfoKHR renderingInfoKHR(VkExtent2D extent, uint32_t colorAttachmentCount, VkRenderingAttachmentInfo* pColorAttachmentInfos, VkRenderingAttachmentInfo* pDepthAttachmentInfo) {
	VkRenderingInfoKHR renderInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.pNext = nullptr,
		.layerCount = 1,
		.colorAttachmentCount = colorAttachmentCount,
		.pColorAttachments = pColorAttachmentInfos,
		.pDepthAttachment = pDepthAttachmentInfo
	};
	renderInfo.renderArea.extent = extent;
	renderInfo.renderArea.offset = { 0, 0 };
	return renderInfo;
}

Frame& Engine::getCurrentFrame() {
	return frames[frameNumber % swapchain.framesInFlight()];
}

void Engine::render() {
	// First, wait for the the last frame to render
	VkFence currentRenderFence = getCurrentFrame().renderFence().handle();
	vkWaitForFences(device.device(), 1, &currentRenderFence, true, 1000000000);
	// Here would be the place to delete all objects from the previous frame (like descriptor sets, etc)
	vkResetFences(device.device(), 1, &currentRenderFence);

	// Next, request current frame's image from the swapchain
	swapchain.acquireNextImage(&getCurrentFrame().presentSemaphore(), nullptr);

	// Get the current frame's command buffer
	Command& cmd = getCurrentFrame().command();
	cmd.reset(); // Reset before adding more commands to be safe
	cmd.begin(); // Begin the command buffer
	
	// Transition the draw image to a writable format
	drawImage.transitionImage(cmd, VK_IMAGE_LAYOUT_GENERAL);
	
	// Now the rendering info struct needs to be filled with the leftover info that the renderpass usually handles
	VkClearValue clearColorValue{ .color{ 0.0f, 0.0f, 0.0f, 1.0f } };
	VkRenderingAttachmentInfoKHR colorAttachmentInfo = Image::attachmentInfo(drawImage.imageView(), &clearColorValue, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingInfoKHR renderingInfo = renderingInfoKHR(window.extent(), 1, &colorAttachmentInfo, nullptr);

	// Transition draw image to a color attachment
	drawImage.transitionImage(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	vkCmdBeginRendering(cmd.buffer(), &renderingInfo);

	// Bind pipelines and draw here
	vkCmdBindPipeline(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline.pipeline()); // Bind pipeline
	// Set dynamic viewport and scissor
	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(window.extent().width),
		.height = static_cast<float>(window.extent().height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	vkCmdSetViewport(cmd.buffer(), 0, 1, &viewport);
	VkRect2D scissor{
		.offset = {0, 0},
		.extent = window.extent()
	};
	vkCmdSetScissor(cmd.buffer(), 0, 1, &scissor);
	vkCmdDraw(cmd.buffer(), 0, 0, 0, 0);

	vkCmdEndRendering(cmd.buffer());

	// Transition images for copying and then presenting
	// Draw image is going to be copied to the swapchain image, so transition it to a transfer source layout
	drawImage.transitionImage(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	// Swapchain image needs to be transitioned to a transfer destination layout
	swapchain.image(swapchain.imageIndex()).transitionImage(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Image::copyImageOnGPU(cmd, drawImage, swapchain.image(swapchain.imageIndex()));

	// Draw ImGui menu here to the swapchain image, but transition the swapchain image to color attachment first

	// Transition swapchain image to a presentation-ready layout
	swapchain.image(swapchain.imageIndex()).transitionImage(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd.end();
	cmd.submitToQueue(device.graphicsQueue(), getCurrentFrame()); // Submit the command buffer
	swapchain.presentToScreen(device.presentQueue(), getCurrentFrame(), swapchain.imageIndex()); // Present to screen
	
	frameNumber++;
}

void Engine::resizeCallback() {
	if (swapchain.resizeRequested()) {
		swapchain.recreate();
		drawImage.recreate({ window.extent().width, window.extent().height, 1 });
	}
}