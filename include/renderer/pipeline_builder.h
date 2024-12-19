#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "device.h"
#include "shader.h"
#include "pipeline.h"
#include <vector>
#include <stdexcept>

class Pipeline;

class PipelineBuilder : public NonCopyable {
public:
	PipelineBuilder(const Device& device);
	// ~PipelineBuilder(); // TODO: Implement a way to delete all pipelines created by the builder. Perhaps using shared pointers?

	// @brief Resets the PipelineBuilder to its default state
	void clear();

	// @brief Build a Pipeline with the current chosen parameters of the PipelineBuilder
	Pipeline buildPipeline();

	// @brief Loads the provided shader modules into the pipeline
	void setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);

	// @brief Sets input topology
	void setInputTopology(VkPrimitiveTopology topology);
	// @brief Sets polygon mode
	void setPolygonMode(VkPolygonMode mode);
	// @brief Sets culling mode
	//
	// @param cullMode - The behavior flags of the culling
	// @param frontFace - Which sides of the triangles are considered the front
	void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	// @brief Sets multisampling level
	void setMultisampling();
	// @brief Disables blending
	void disableBlending();
	// @brief Sets the format of the color attachment
	void setColorAttachmentFormat(VkFormat format);
	// @brief Sets the depth attachment format
	void setDepthAttachmentFormat(VkFormat format);
	// @brief Sets depth testing using provided compare op
	//
	// @param compareOp - If compareOp is provided, enable depth testing using specified compare operation. Otherwise, disable depth testing
	void setDepthTest(VkCompareOp compareOp = VK_COMPARE_OP_NEVER);

	// @brief Sets the pipeline layout. If a create info is supplied instead, create one and set the private variable to the newly created layout.
	void setPipelineLayout(VkPipelineLayout layout);
	void setPipelineLayout(VkPipelineLayoutCreateInfo layoutInfo);

	// @brief Sets the vertex input state
	void setVertexInputState(VkPipelineVertexInputStateCreateInfo createInfo);

	// Creates a pipeline layout using the given create info
	static VkPipelineLayout createPipelineLayout(const Device& device, VkPipelineLayoutCreateInfo createInfo);

	// @brief Create a default, blank VkPipelineLayoutCreateInfo struct
	static VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& setLayouts = {}, const std::vector<VkPushConstantRange>& pushConstantRanges = {});
	// @brief Create a default, blank VkPipelineVertexInputStateCreateInfo struct (probably won't need this when I implement buffer device address functionality)
	static VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo();

private:
	// @brief Reference to the Vulkan device which creates the pipelines
	const Device& _device;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo; // Removing this because of a better vertex indexing system

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineRenderingCreateInfo renderingInfo;
	VkPipelineLayout pipelineLayout;
	VkFormat colorAttachmentFormat;
};