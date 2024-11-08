#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "device.h"
#include <span>
#include <unordered_map>
#include <vector>
#include <string>

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

	inline VkDescriptorPool pool() const { return _descriptorPool; }

private:
	const Device& _device;
	VkDescriptorPool _descriptorPool;
};

class DescriptorLayoutBuilder : public NonCopyable {
public:
	DescriptorLayoutBuilder(const Device& device);
	~DescriptorLayoutBuilder();

	// @brief Adds a binding and descriptor type to the descriptor layout builder
	// 
	// @param binding - Which binding position to assign this to
	// @param descriptorType - Which type of descriptor set to bind
	// @param shaderStages - Which shader will use this descriptor set
	void addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);

	// @brief Clears the builder of current bindings
	void clear();

	// @brief Builds a descriptor set layout with the current bindings
	VkDescriptorSetLayout build();

	// @brief Builds a descriptor set layout and adds it to _descriptorLayouts
	//
	// @param name - name to map the layout with
	void build(const std::string& name);

	inline VkDescriptorSetLayout get(const std::string& name) const { return _descriptorLayouts.at(name); }

private:
	const Device& _device;

	std::vector<VkDescriptorSetLayoutBinding> _bindings;
	std::unordered_map<std::string, VkDescriptorSetLayout> _descriptorLayouts;
};

// TODO: How to store descriptor sets without creating a separate DescriptorSet class for each descriptor set?
// First idea below:
class DescriptorSets : public NonCopyable {
public:
	// In the constructor, initialize the allocator and prepare everything to be able to create the sets
	DescriptorSets(const Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios);

	// @brief Clears the currently allocated descriptor sets
	void clearDescriptorSets();
	
	// @brief Allocates a descriptor set and adds it to _descriptorSets
	//
	// @param layout - Descriptor set layout to create the descriptor set with
	void allocateDescriptorSet(const std::string& name, VkDescriptorSetLayout layout);

	inline VkDescriptorSet get(const std::string& name) const { return _descriptorSets.at(name); }

private:
	const Device& _device;

	DescriptorPool _descriptorPool;
	std::unordered_map<std::string, VkDescriptorSet> _descriptorSets;
};



