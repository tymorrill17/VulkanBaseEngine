#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "device.h"
#include "utility/allocator.h"


class Buffer : public NonCopyable {
public:
	Buffer(const Device& device, const Allocator& allocator, size_t instanceSize,
		uint32_t instanceCount, VkBufferUsageFlags usageFlags,
		VmaMemoryUsage memoryUsage, size_t minOffsetAlignment = 1);
	~Buffer();

	// @brief Maps CPU-accessible pointer to the buffer on the GPU
	void map();
	// @brief Unmap the CPU-accessible pointer
	void unmap();

	// @brief Writes data to the buffer. The data written is either the entire capacity, or a specified size and offset
	//
	// @param data - The data to be written to the buffer
	// @param size - The size of the data to be written
	// @param offset - Amount to offset the writing in the buffer
	void writeBuffer(void* data, size_t size = VK_WHOLE_SIZE, size_t offset = 0);
	// @brief Writes an instance of the buffer's data at the index
	//
	// @param data - The data to be written to the buffer
	// @param index - Which instance to write to
	void writeBufferAtIndex(void* data, int index);

	inline VkBuffer buffer() const { return _buffer; }
	inline VmaAllocation allocation() const { return _allocation; }
	inline VmaAllocationInfo allocationInfo() const { return _allocationInfo; }
	inline size_t bufferSize() const { return _bufferSize; }
	inline uint32_t instanceCount() const { return _instanceCount; }
	inline size_t instanceSize() const { return _instanceSize; }
	inline size_t alignmentSize() const { return _alignmentSize; }

private:
	const Device& _device;
	const Allocator& _allocator;

	VkBuffer _buffer; // Vulkan buffer object
	VmaAllocation _allocation; // vma allocation object
	VmaAllocationInfo _allocationInfo; // Info used to allocate the buffer with vma

	void* _mappedData;

	size_t _bufferSize; // The total size in bytes of the buffer
	uint32_t _instanceCount; // How many instances of the struct being stored by the buffer (usually the number of frames in flight)
	size_t _instanceSize; // The size in bytes of a single instance of the struct being stored by the buffer
	size_t _alignmentSize; // The device-specific alignment size

	static size_t findAlignmentSize(size_t instanceSize, size_t minOffsetAlignment);
};