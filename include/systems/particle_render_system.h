#pragma once

#include "vulkan/vulkan.h"
#include "render_system.h"
#include "renderer/renderer.h"
#include "renderer/device.h"
#include "renderer/descriptor.h"
#include "renderer/pipeline_builder.h"
#include "renderer/command.h"

#include <vector>

class ParticleRenderSystem : public RenderSystem {
public:
	ParticleRenderSystem(Renderer& renderer);
	~ParticleRenderSystem();

	void render();

private:
	Renderer& _renderer;

	Pipeline _defaultPipeline;

	DescriptorPool _globalDescriptorPool;
	VkDescriptorSet _particleDescriptor;
};