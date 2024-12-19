#pragma once

#include "vulkan/vulkan.h"
#include "render_system.h"
#include "renderer/renderer.h"
#include "renderer/device.h"
#include "renderer/pipeline_builder.h"
#include "renderer/command.h"

#include <vector>

class ParticleRenderSystem : public RenderSystem {
public:
	ParticleRenderSystem(Renderer& renderer, std::vector<VkDescriptorSetLayout> particleDescriptorLayout, std::vector<VkDescriptorSet> particleDescriptorSets, int numParticles);

	void render();

	void bindDescriptor(VkDescriptorSet set);

private:
	Renderer& _renderer;

	Pipeline _pipeline;

	std::vector<VkDescriptorSetLayout> _particleDescriptors;
	std::vector<VkDescriptorSet> _particleSet;


	const int _numParticles;

	void buildPipeline();
};