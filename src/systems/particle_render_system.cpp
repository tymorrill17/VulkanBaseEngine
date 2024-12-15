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

void ParticleRenderSystem::buildPipeline() {

	_renderer.pipelineBuilder().clear();

	std::string baseDir = static_cast<std::string>(BASE_DIR);
	std::string folderDir = baseDir + "\\shaders\\";

	VkShaderModule defaultVertShader;
	Shader::loadShaderModule(folderDir + "circle.vert.spv", _renderer.device(), defaultVertShader);
	VkShaderModule defaultFragShader;
	Shader::loadShaderModule(folderDir + "circle.frag.spv", _renderer.device(), defaultFragShader);
	_renderer.pipelineBuilder().setShaders(defaultVertShader, defaultFragShader);

	VkPipelineLayout layout = PipelineBuilder::createPipelineLayout(_renderer.device(), PipelineBuilder::pipelineLayoutCreateInfo());
	_renderer.pipelineBuilder().setPipelineLayout(layout);
	_renderer.pipelineBuilder().setVertexInputState(PipelineBuilder::vertexInputStateCreateInfo());

	_renderer.pipelineBuilder().setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	_renderer.pipelineBuilder().setPolygonMode(VK_POLYGON_MODE_FILL);
	_renderer.pipelineBuilder().setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	_renderer.pipelineBuilder().setMultisampling();
	_renderer.pipelineBuilder().disableBlending();
	_renderer.pipelineBuilder().setDepthTest();
	_renderer.pipelineBuilder().setColorAttachmentFormat(_renderer.swapchain().imageFormat());

	_pipeline = _renderer.pipelineBuilder().buildPipeline();
}

ParticleRenderSystem::ParticleRenderSystem(Renderer& renderer) :
	_renderer(renderer), 
	_globalDescriptorPool(_renderer.device(), 10, renderDescriptorSetSizes) {

	buildPipeline();

	// Set up descriptor sets here
	
	VkDescriptorSetLayout particleDescriptor = _renderer.descriptorLayoutBuilder()
		.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();
	_particleDescriptor = _globalDescriptorPool.allocateDescriptorSet(particleDescriptor);
}

void ParticleRenderSystem::render() {
	// Get the current frame's command buffer
	Command& cmd = _renderer.getCurrentFrame().command();

	// Bind pipelines and draw here
	vkCmdBindPipeline(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipeline()); // Bind pipeline

	vkCmdDraw(cmd.buffer(), 0, 0, 0, 0);
}