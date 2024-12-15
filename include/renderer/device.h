#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include "instance.h"
#include "utility/window.h"
#include "queue_family.h"
#include <vector>
#include <string>
#include <set>

class Device : public NonCopyable {
public:
	Device(const Instance& instance, Window& window, const std::vector<const char*>& extensions);
	~Device();

	static VkPhysicalDeviceFeatures deviceFeatures;
	static VkPhysicalDeviceVulkan12Features features12;
	static VkPhysicalDeviceVulkan13Features features13;

	// @brief Get the physicial device Vulkan object
	inline VkPhysicalDevice physicalDevice() const { return _physDevice; }
	// @brief Get the physicial device properties
	inline VkPhysicalDeviceProperties physicalDeviceProperies() const { return _physDeviceProperties; }
	// @brief Get the logical device Vulkan object
	inline VkDevice device() const { return _logicalDevice; }
	// @brief Get the queue family indices object
	inline QueueFamilyIndices queueFamilyIndices() const { return _indices; }
	// @brief Get the graphics queue Vulkan object
	inline VkQueue graphicsQueue() const { return _graphQueue; }
	// @brief Get the present queue Vulkan object
	inline VkQueue presentQueue() const { return _presQueue; }

private:
	// @brief Representation of the physical GPU
	VkPhysicalDevice _physDevice;
	// @brief Properties of the chosen GPU
	VkPhysicalDeviceProperties _physDeviceProperties;

	// @brief Logical representation of the physical device that the code can interact with
	VkDevice _logicalDevice;

	const Instance& _instance;
	Window& _window;

	QueueFamilyIndices _indices;
	VkQueue _graphQueue; // Graphics queue
	VkQueue _presQueue; // Present queue

	// @brief Verify that the selected physical device supports the requested extensions
	//
	// @param physicalDevice - The selected physical device to check
	// @param extensions - The requested device extensions
	// @return True if the physical device supports all of extensions. False otherwise
	static bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*>& extensions);
	
	// @brief Queries available physical devices and selects one based on whether or not it supports required device extensions
	//
	// @param instance - The current active instance of Vulkan
	// @param surface - The surface which the swapchain will present to
	// @param requiredExtensions - The device extensions that are required to present images to the screen
	// @return The selected VkPhysicalDevice object
	static VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);

	// @brief Checks whether the selected physical device has swapchain support
	//
	// @param physicalDevice - The selected physical device to check
	// @param surface - The surface which the swapchain will present to
	// @return True if the physical device has swapchain support. False otherwise 
	static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};