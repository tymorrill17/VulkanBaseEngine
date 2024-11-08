#pragma once
#include <vulkan/vulkan.h>
#include "renderer/queue_family.h"
#include <string>
#include <vector>
#include <iostream>

// @brief Manages debugging messaging and console logging
class Logger {
public:

	// @brief Get the static logger instance, or create one if non-existent
	//
	// @return Pointer to the logger
	static Logger* get_logger();

	// @brief Shut down and delete the logger
	static void shutdown();

	// @brief Set whether the logger is active or not
	//
	// @param isActive - Enable (true) or disable (false) the logger.
	void set_active(bool isActive);

	// @brief Return the logger's status
	//
	// @return Whether or not the logger is active
	bool is_active() const;

	// @brief Print a message to the console
	//
	// @param message - Message to be printed
	void print(std::string message) const;

	// @brief Print a list of strings
	void print_list(std::vector<const char*>& list) const;

	// @brief Print a list of validation layers
	void print_layers(const char* layerCategory, std::vector<VkLayerProperties>& layers) const;
	void print_layers(const char* layerCategory, std::vector<const char*>& layers) const;

	// @brief Print a list of extensions
	void print_extensions(const char* extensionCategory, std::vector<VkExtensionProperties>& extensions) const;
	void print_extensions(const char* extensionCategory, std::vector<const char*>& extensions) const;

	// @brief Print a list of physical devices
	void print_devices(std::vector<VkPhysicalDevice>& devices) const;

	// @brief Print the Vulkan version number
	void report_version(uint32_t version) const;

	// @brief Print details about the QueueFamilyIndices
	void log(struct QueueFamilyIndices& indices) const;
	// @brief Print details about the physical device
	void log(struct VkPhysicalDeviceProperties& physDevice) const;

private:

	// @brief The static logger instance
	static Logger* logger;

	bool active;
};

