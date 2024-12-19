#include "application/application.h"

static std::vector<PoolSizeRatio> renderDescriptorSetSizes = {
	//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
	{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
};

static std::vector<PoolSizeRatio> computeDescriptorSetSizes = {
	{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
};

constexpr int NUM_PARTICLES = 1;

struct ParticleInfo {
	Particle2D particles[NUM_PARTICLES];
	glm::vec3 defaultColor;
	float radius;
};

Application::Application() : 
	window({ APPLICATION_WIDTH, APPLICATION_HEIGHT }, "VulkanEngineV2"),
	renderer(window) {

	// Get static logger
	logger = Logger::get_logger();
}

void Application::run() {
	// Initialize the descriptor pool
	DescriptorPool globalDescriptorPool(renderer.device(), 10, renderDescriptorSetSizes);

	glm::vec3 particleColor{ 1.0f, 1.0f, 1.0f };
	float particleRadius = 1.0f;

	// The particle info struct contains the Particle struct (pos and vel), as well as color and radius of each particle
	ParticleInfo particleInfo{
	.defaultColor = particleColor,
	.radius = particleRadius
	};

	// The constructor of the particle system initializes the positions of the particles to a grid
	ParticleSystem2D fluidParticles(NUM_PARTICLES, particleRadius, particleInfo.particles);

	// We will use a storage buffer for the particle info since we want to be able to send many particles and don't want to be limited by capacity
	Buffer particleBuffer(renderer.device(), renderer.allocator(), sizeof(ParticleInfo), renderer.swapchain().framesInFlight(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, renderer.device().physicalDeviceProperies().limits.minStorageBufferOffsetAlignment);
	particleBuffer.map();

	std::vector<VkDescriptorSetLayout> particleLayouts = { renderer.descriptorLayoutBuilder().addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build() };
	VkDescriptorSet particleDescriptor = globalDescriptorPool.allocateDescriptorSet(particleLayouts[0]);
	renderer.descriptorWriter().writeBuffer(0, particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).updateDescriptorSet(particleDescriptor);

	// Create the render systems and add them to the renderer
	ParticleRenderSystem particleRenderSystem(renderer, particleLayouts, std::vector<VkDescriptorSet>{particleDescriptor}, NUM_PARTICLES);
	renderer.addRenderSystem(&particleRenderSystem);
	
	// Main application loop
	while (!window.shouldClose()) {
		window.process_inputs(); // Poll the user inputs
		if (window.pauseRendering()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		//particleSystem.update(); // Update the particle systems

		// Update/fill buffers
		particleBuffer.writeBuffer(&particleInfo);

		renderer.renderAll(); // Have the renderer render all the render systems

		renderer.resizeCallback(); // Check for window resize and call the window resize callback function
	}
	logger->print("Shutting Down... Bye Bye!");
}
