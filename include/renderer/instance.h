#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/window.h"
#include "logger/logger.h"
#include "logger/debug_messenger.h"
#include <vector>

class Instance : NonCopyable {
public:
	Instance(const char* appName, const char* engineName, bool enableValidationLayers);
	~Instance();

	// @brief Get the handle of the VkInstance object
	//
	// @return The reference of the VkInstance object
	inline VkInstance handle() const { return instance; }

	// @brief Are validation layers enabled for the instance
	inline bool validationLayersEnabled() const { return enableValidationLayers; }

	// @brief Requested validation layers to enable
	static std::vector<const char*> validationLayers;

	// @brief Requested device extensions to use
	static std::vector<const char*> deviceExtensions;

private:

	// @brief Debug logger
	Logger* logger;

	// @brief The Vulkan instance object.
	VkInstance instance;

	// @brief Should validation layers be enabled.
	bool enableValidationLayers;

	// @brief Verify that the instance supports the requested validation layers.
	//
	// @return True if requested validation layers in Instance::validationLayers are supported. False if not.
	static bool checkValidationLayerSupport();

	// @brief Queries the window system for API-specific instance extensions that are needed. Also enabled validation layer extension if validation layers are enabled.
	//
	// @param extensions - Required extension names are filled here.
	// @param validationLayers - Validation layers enabled or not?
	static void getRequiredInstanceExtensions(std::vector<const char*>& extensions, bool validationLayers);

};