#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/allocator.h"
#include "logger/debug_messenger.h"
#include "shader.h"
#include "pipeline_builder.h"
#include "swapchain.h"
#include "image.h"
#include "descriptor.h"
#include <string>

class Swapchain;
class AllocatedImage;

class Engine : NonCopyable {
public:
	// @brief Construct and initialize the Vulkan 
	Engine(Window& window);

	// @brief Destroy engine instance and clean up allocations
	~Engine() = default;

	// @brief Renders the scene and presents it to the surface
	void render();

	// @brief Handles changes that need to be made when the window is resized
	void resizeCallback();


private:
	Logger* logger; // Debug logger
	Window& window; // Main window to render to
	Instance instance;
	DebugMessenger debugMessenger; // Vulkan debug messenger callback for validation layers
	Device device; // Device object containing physical and logical devices
	Allocator allocator; // Allocator for buffers and images
	Swapchain swapchain; // The swapchain handles presenting images to the surface and thus to the window
	PipelineBuilder pipelineBuilder; // Pipeline builder object that abstracts and handles pipeline creation
	std::vector<Frame> frames; // Contains command buffers and sync objects for each frame in the swapchain
	uint32_t frameNumber; // Keeps track of the number of rendered frames
	AllocatedImage drawImage; // Image that gets rendered to then copied to the swapchain image
	DescriptorLayoutBuilder descriptorLayoutBuilder; // Builds descriptor set layouts
	DescriptorSets renderDescriptorSets; // Keeps track of all descriptor sets related to rendering
	DescriptorSets computeDescriptorSets; // Keeps track of all compute-related descriptor sets

	Pipeline defaultPipeline;
	
	// @brief Gets the current frame by finding frameNumber % swapchain.framesInFlight
	Frame& getCurrentFrame();
};