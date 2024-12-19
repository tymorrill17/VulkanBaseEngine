#include "systems/particle_render_system.h"

void ParticleRenderSystem::buildPipeline() {

	_renderer.pipelineBuilder().clear();

	std::string baseDir = static_cast<std::string>(BASE_DIR);
	std::string folderDir = baseDir + "\\shaders\\";

	VkShaderModule defaultVertShader;
	Shader::loadShaderModule(folderDir + "circle.vert.spv", _renderer.device(), defaultVertShader);
	VkShaderModule defaultFragShader;
	Shader::loadShaderModule(folderDir + "circle.frag.spv", _renderer.device(), defaultFragShader);
	_renderer.pipelineBuilder().setShaders(defaultVertShader, defaultFragShader);

	VkPipelineLayout layout = PipelineBuilder::createPipelineLayout(_renderer.device(), PipelineBuilder::pipelineLayoutCreateInfo(_particleDescriptors));
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

ParticleRenderSystem::ParticleRenderSystem(Renderer& renderer, std::vector<VkDescriptorSetLayout> particleDescriptorLayout, std::vector<VkDescriptorSet> particleDescriptorSets, int numParticles) :
	_renderer(renderer), 
	_particleDescriptors(particleDescriptorLayout),
	_particleSet(particleDescriptorSets),
	_numParticles(numParticles) {

	buildPipeline();
}

void ParticleRenderSystem::render() {
	// Get the current frame's command buffer
	Command& cmd = _renderer.getCurrentFrame().command();

	// Bind pipelines and draw here
	vkCmdBindPipeline(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipeline()); // Bind pipeline

	vkCmdBindDescriptorSets(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipelineLayout(), 0, _particleSet.size(), _particleSet.data(), 0, nullptr);

	vkCmdDraw(cmd.buffer(), 6 * _numParticles, 0, 0, 0);
}