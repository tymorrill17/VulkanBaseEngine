#include "renderer/descriptor.h"

DescriptorPool::DescriptorPool(const Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios) : 
	_device(device), _descriptorPool(VK_NULL_HANDLE) {

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolSizeRatios) {
		poolSizes.emplace_back(ratio.type, static_cast<uint32_t>(ratio.ratio * maxSets));
	}
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.maxSets = maxSets,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};
	if (vkCreateDescriptorPool(_device.device(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(_device.device(), _descriptorPool, nullptr);
}

DescriptorLayoutBuilder::DescriptorLayoutBuilder(const Device& device) : _device(device) {}

DescriptorLayoutBuilder::~DescriptorLayoutBuilder() {
	for (auto& [name, layout] : _descriptorLayouts) {
		vkDestroyDescriptorSetLayout(_device.device(), layout, nullptr);
	}
}

void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding newBinding{
		.binding = binding,
		.descriptorType = descriptorType,
		.descriptorCount = 1,
		.stageFlags = stageFlags
	};
	_bindings.push_back(newBinding);
}

void DescriptorLayoutBuilder::clear() {
	_bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build() {
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(_bindings.size()),
		.pBindings = _bindings.data()
	};
	VkDescriptorSetLayout layout;
	if (vkCreateDescriptorSetLayout(_device.device(), &descriptorSetLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to build descriptor set layout!");
	}
	return layout;
}

void DescriptorLayoutBuilder::build(const std::string& name) {
	VkDescriptorSetLayout layout = build();
	_descriptorLayouts[name] = layout;
}

DescriptorSets::DescriptorSets(const Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios) : 
	_device(device), _descriptorPool(device, maxSets, poolSizeRatios) {}

void DescriptorSets::clearDescriptorSets() {
	vkResetDescriptorPool(_device.device(), _descriptorPool.pool(), 0);
}

void DescriptorSets::allocateDescriptorSet(const std::string& name, VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = _descriptorPool.pool(),
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};
	VkDescriptorSet set;
	if (vkAllocateDescriptorSets(_device.device(), &allocInfo, &set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}
	_descriptorSets[name] = set;
}
