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
#include "systems/render_system.h"
#include <string>

class Swapchain;
class AllocatedImage;

class Renderer : public NonCopyable {
public:
	// @brief Construct and initialize the Vulkan 
	Renderer(Window& window);

	// @brief Destroy engine instance and clean up allocations
	~Renderer() = default;

	// @brief Renders each RenderSystem to the frame and presents it
	void renderAll();

	// @brief Handles changes that need to be made when the window is resized
	void resizeCallback();

	// @brief Adds renderSystem to the end of the renderSystems list
	// 
	// @param renderSystem - render system to add to the Renderer's list
	// @return Returns the Renderer handle in order to chain together adds
	Renderer& addRenderSystem(RenderSystem* renderSystem);

	// @brief Gets the current frame by finding frameNumber % swapchain.framesInFlight
	Frame& getCurrentFrame();

	const inline Device& device() const { return _device; }
	inline Swapchain& swapchain() { return _swapchain; }
	inline PipelineBuilder& pipelineBuilder() { return _pipelineBuilder; }
	inline DescriptorLayoutBuilder& descriptorLayoutBuilder() { return _descriptorLayoutBuilder; }
	inline DescriptorWriter& descriptorWriter() { return _descriptorWriter; }
	const inline Allocator& allocator() const { return _allocator; }

private:
	Logger* _logger; // Debug logger
	Window& _window; // Main window to render to
	Instance _instance;
	DebugMessenger _debugMessenger; // Vulkan debug messenger callback for validation layers
	Device _device; // Device object containing physical and logical devices
	Allocator _allocator; // Allocator for buffers and images
	Swapchain _swapchain; // The swapchain handles presenting images to the surface and thus to the window
	PipelineBuilder _pipelineBuilder; // Pipeline builder object that abstracts and handles pipeline creation
	std::vector<Frame> _frames; // Contains command buffers and sync objects for each frame in the swapchain
	uint32_t _frameNumber; // Keeps track of the number of rendered frames
	AllocatedImage _drawImage; // Image that gets rendered to then copied to the swapchain image
	DescriptorLayoutBuilder _descriptorLayoutBuilder; // Builds descriptor set layouts
	DescriptorWriter _descriptorWriter;

	std::vector<RenderSystem*> _renderSystems;
};