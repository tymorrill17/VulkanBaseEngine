#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "device.h"
#include "command.h"
#include "sync.h"

class Command;

class Frame : NonCopyable {
public:
	Frame(const Device& device);

	Frame(Frame&& other) noexcept;
	Frame& operator=(Frame&& other) noexcept;

	inline Semaphore& presentSemaphore() { return _presentSemaphore; }
	inline Semaphore& renderSemaphore() { return _renderSemaphore; }
	inline Fence& renderFence() { return _renderFence; }
	inline Command& command() { return _command; }

private:
	const Device& _device;
	Semaphore _presentSemaphore;
	Semaphore _renderSemaphore;
	Fence _renderFence;
	Command _command;
};
