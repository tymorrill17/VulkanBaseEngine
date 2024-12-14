#include "application/application.h"

constexpr int NUM_PARTICLES = 1;

struct ParticleInfo {
	Particle2D particles[NUM_PARTICLES];
	glm::vec3 color;
	float radius;
};

Application::Application() : 
	window({ APPLICATION_WIDTH, APPLICATION_HEIGHT }, "VulkanEngineV2"),
	renderer(window) {

	// Get static logger
	logger = Logger::get_logger();
}

void Application::run() {
	glm::vec3 particleColor{ 1.0f, 1.0f, 1.0f };
	float particleRadius = 1.0f;

	ParticleInfo particleBuffer{
	.color = particleColor,
	.radius = particleRadius
	};

	ParticleSystem2D fluidParticles(NUM_PARTICLES, particleRadius, particleBuffer.particles);

	ParticleRenderSystem particleRenderSystem(renderer);
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


		renderer.drawFrame(); // Run the render system
		renderer.resizeCallback(); // Check for window resize and call the window resize callback function
	}
	logger->print("Shutting Down... Bye Bye!");
}
