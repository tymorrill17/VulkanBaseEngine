#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "device.h"

class Pipeline : NonCopyable {
public:
	Pipeline();
	Pipeline(const Device* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);
	~Pipeline();

	// Write move constructors for the pipeline builder to function properly
	Pipeline(Pipeline&& other) noexcept;
	Pipeline& operator=(Pipeline&& other) noexcept;

	inline const VkPipeline pipeline() const { return _pipeline; }
	inline const VkPipelineLayout pipelineLayout() const { return _pipelineLayout; }

private:
	// @brief The Vulkan render pipeline object
	VkPipeline _pipeline;
	// @brief The pipeline layout used for interacting with the pipeline
	VkPipelineLayout _pipelineLayout;

	// @brief Reference to the Vulkan device used to create the pipeline
	const Device* _device;

	void cleanup();
};