#pragma once
#include <vulkan/vulkan.h>

#include "instance.h"
#include "device.h"
#include "frame.h"
#include "queue_family.h"
#include "NonCopyable.h"
#include "image.h"

#include <vector>
#include <sstream>

class Device;
class Window;
class SwapchainImage;

// @brief Stores supported swapchain attributes
struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain : public NonCopyable {
public:
	Swapchain(const Device& device, Window& window);
	~Swapchain();

	// @brief Recreates the swapchain as a result of window resizing
	void recreate();

	inline VkSwapchainKHR handle() const { return _swapchain; }
	inline VkFormat imageFormat() const { return _imageFormat; }
	inline VkExtent2D extent() const { return _extent; }
	inline size_t imageCount() const { return _images.size(); }
	inline uint32_t imageIndex() const { return _imageIndex; }
	inline SwapchainSupportDetails supportDetails() const { return _supportDetails; }
	inline SwapchainImage& image(uint32_t index) { return _images[index]; }
	inline uint32_t framesInFlight() const { return _framesInFlight; }
	inline bool resizeRequested() const { return _resizeRequested; }

	void acquireNextImage(Semaphore* semaphore, Fence* fence);
	void presentToScreen(VkQueue queue, Frame& frame, uint32_t imageIndex);

	// STATIC METHODS

	// @brief Queries swapchain support attributes
	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	// @brief Sets the swapchain extent based on the current window size
	static VkExtent2D setSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities, const Window& window);

private:
	const Device& _device;
	Window& _window;

	// @brief Supported swapchain attributes
	SwapchainSupportDetails _supportDetails;
	VkSwapchainKHR _swapchain;

	// @brief Swapchain images
	std::vector<SwapchainImage> _images;

	// @brief Format of the swapchain images
	VkFormat _imageFormat;
	// @brief Extent of the swapchain image views
	VkExtent2D _extent;
	// @brief How many frames the swapchain contains and can be rendered in parallel
	uint32_t _framesInFlight;
	// @brief The index of the current swapchain image being rendered to
	uint32_t _imageIndex;

	bool _resizeRequested; 

	void createSwapchain();
	void cleanup();

	static VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};