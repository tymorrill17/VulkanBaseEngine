#include "systems/particle_render_system.h"

static std::vector<PoolSizeRatio> renderDescriptorSetSizes = {
	//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
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

ParticleRenderSystem::ParticleRenderSystem(Renderer& renderer) :
	_renderer(renderer), 
	_globalDescriptorPool(_renderer.device(), 10, renderDescriptorSetSizes) {

	_defaultPipeline = buildDefaultPipeline(_renderer.device(), _renderer.pipelineBuilder(), _renderer.swapchain());

	// Set up descriptor sets here
	
	VkDescriptorSetLayout particleDescriptor = _renderer.descriptorLayoutBuilder()
		.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();
	_particleDescriptor = _globalDescriptorPool.allocateDescriptorSet(particleDescriptor);
}

ParticleRenderSystem::~ParticleRenderSystem() {

}

void ParticleRenderSystem::render() {
	// Get the current frame's command buffer
	Command& cmd = _renderer.getCurrentFrame().command();

	// Bind pipelines and draw here
	vkCmdBindPipeline(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _defaultPipeline.pipeline()); // Bind pipeline

	vkCmdDraw(cmd.buffer(), 0, 0, 0, 0);
}