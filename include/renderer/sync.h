#pragma once
#include "vulkan/vulkan.h"
#include "device.h"
#include "NonCopyable.h"

class Semaphore : public NonCopyable {
public:
	Semaphore(const Device& device, VkSemaphoreCreateFlags flags = 0U);
	~Semaphore();

	Semaphore(Semaphore&& other) noexcept;
	Semaphore& operator=(Semaphore&& other) noexcept;

	inline VkSemaphore handle() const { return _semaphore; }

	void cleanup();

private:
	const Device& _device;
	VkSemaphore _semaphore;
	VkSemaphoreCreateFlags _flags;
};

class Fence : public NonCopyable {
public:
	Fence(const Device& device, VkFenceCreateFlags flags = 0U);
	~Fence();

	Fence(Fence&& other) noexcept;
	Fence& operator=(Fence&& other) noexcept;

	inline VkFence handle() const { return _fence; }

	void cleanup();

private:
	const Device& _device;
	VkFence _fence;
	VkFenceCreateFlags _flags;
};