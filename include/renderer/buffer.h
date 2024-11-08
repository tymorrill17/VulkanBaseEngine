#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "device.h"
#include "utility/allocator.h"

class AllocatedBuffer : public NonCopyable {
public:
	AllocatedBuffer(const Device& device, Allocator& allocator,
		size_t allocationSize, VkBufferUsageFlags usageFlags,
		VmaMemoryUsage memoryUsage);
	~AllocatedBuffer() { cleanup(); }

	inline VkBuffer buffer() const { return _buffer; }
	inline VmaAllocation allocation() const { return _allocation; }
	inline VmaAllocationInfo allocationInfo() const { return _allocationInfo; }

private:
	VkBuffer _buffer;
	VmaAllocation _allocation;
	VmaAllocationInfo _allocationInfo;

	const Device& _device;
	const Allocator& _allocator;

	void cleanup();
};