#include "renderer/buffer.h"

Buffer::Buffer(const Device& device, const Allocator& allocator, size_t instanceSize,
	uint32_t instanceCount, VkBufferUsageFlags usageFlags,
	VmaMemoryUsage memoryUsage, size_t minOffsetAlignment) :
	_device(device),
	_allocator(allocator),
	_buffer(VK_NULL_HANDLE),
	_mappedData(nullptr),
	_bufferSize(instanceSize*instanceCount),
	_instanceCount(instanceCount),
	_instanceSize(instanceSize) {
	
	_alignmentSize = findAlignmentSize(_instanceSize, minOffsetAlignment);

	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = _bufferSize,
		.usage = usageFlags
	};

	VmaAllocationCreateInfo allocationCreateInfo{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage
	};

	if (vmaCreateBuffer(_allocator.handle(), &bufferCreateInfo, &allocationCreateInfo, &_buffer, &_allocation, &_allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create allocated buffer!");
	}
}

Buffer::~Buffer() {
	unmap();
	vmaDestroyBuffer(_allocator.handle(), _buffer, _allocation);
}

void Buffer::map() {
	if (vmaMapMemory(_allocator.handle(), _allocation, &_mappedData) != VK_SUCCESS) {
		throw std::runtime_error("Failed to map memory to the buffer!");
	}
}

void Buffer::unmap() {
	if (_mappedData) {
		vmaUnmapMemory(_allocator.handle(), _allocation);
		_mappedData = nullptr;
	}
}

void Buffer::writeBuffer(void* data, size_t size, size_t offset) {
	if (!_mappedData) {
		throw std::runtime_error("Trying to write to an unmapped buffer!");
	}

	if (size == VK_WHOLE_SIZE) {
		memcpy(_mappedData, data, _bufferSize);
	} else {
		char* memOffset = (char*)_mappedData;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

void Buffer::writeBufferAtIndex(void* data, int index) {
	writeBuffer(data, _instanceSize, index * _alignmentSize);
}

size_t Buffer::findAlignmentSize(size_t instanceSize, size_t minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}