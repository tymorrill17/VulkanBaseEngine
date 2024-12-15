#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "device.h"
#include "buffer.h"
#include "image.h"
#include <span>
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>

// @brief Describes how many of each type of descriptor set to make room for in the descriptor pool.
//		  Used in the initialization of the descriptor pool in the DescriptorAllocator.
struct PoolSizeRatio {
	VkDescriptorType type;
	float ratio;
};

class DescriptorPool : public NonCopyable {
public:
	DescriptorPool(const Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios);
	~DescriptorPool();

	// @brief Allocates a descriptor set using layout
	//
	// @param layout - Descriptor set layout to create the descriptor set with
	VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);

	// @brief Clears the currently allocated descriptor sets
	void clearDescriptorSets();

	inline VkDescriptorPool pool() const { return _descriptorPool; }

private:
	const Device& _device;
	VkDescriptorPool _descriptorPool;
};

class DescriptorLayoutBuilder : public NonCopyable {
public:
	DescriptorLayoutBuilder(const Device& device);

	// @brief Adds a binding and descriptor type to the descriptor layout builder
	// 
	// @param binding - Which binding position to assign this to
	// @param descriptorType - Which type of descriptor set to bind
	// @param shaderStages - Which shader will use this descriptor set
	// @return The handle of the builder for chaining
	DescriptorLayoutBuilder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);

	// @brief Clears the builder of current bindings
	void clear();

	// @brief Builds a descriptor set layout with the current bindings
	VkDescriptorSetLayout build();

private:
	const Device& _device;

	std::vector<VkDescriptorSetLayoutBinding> _bindings;
};

// DescriptorWriter is for binding and writing the data to the GPU
class DescriptorWriter : public NonCopyable{
public:
	DescriptorWriter(const Device& device);

	// @brief adds a VkDescriptorImageInfo to the imageInfos queue to be written using updateSet()
	void writeImage(uint32_t binding, AllocatedImage& image, VkSampler sampler, VkDescriptorType descriptorType);

	// @brief adds a VkDescriptorBufferInfo to the bufferInfos queue to be written using updateSet()
	void writeBuffer(uint32_t binding, Buffer& buffer, size_t bufferSize, size_t offset, VkDescriptorType descriptorType);

	// @brief clears the imageInfos, bufferInfos, and writes
	void clear();

	// @brief updates and writes the set using the infos in _imageInfos and _bufferInfos
	void updateDescriptorSet(VkDescriptorSet descriptor);

private:
	const Device& _device;

	std::deque<VkDescriptorImageInfo> _imageInfos;
	std::deque<VkDescriptorBufferInfo> _bufferInfos;
	std::vector<VkWriteDescriptorSet> _writes;

};



