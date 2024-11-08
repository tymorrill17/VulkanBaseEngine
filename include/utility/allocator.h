#pragma once
#include "vma/vk_mem_alloc.h"
#include "renderer/device.h"
#include "renderer/instance.h"
#include "NonCopyable.h"

class Allocator : NonCopyable {
public:
	Allocator(Device& device, Instance& instance);
	~Allocator();

	inline static const Allocator* allocator() { 
		if (!_allocator) {
			throw std::runtime_error("Trying to get VMA Allocator but is has not been initialized!");
		}
		return _allocator; 
	}
	inline VmaAllocator handle() const { return _vmaAllocator; }

private:
	static Allocator* _allocator;

	// @brief The actual VMA allocator instance
	VmaAllocator _vmaAllocator;

	const Device& _device;
	const Instance& _instance;
};