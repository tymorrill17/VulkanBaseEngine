#include "renderer/device.h"

VkPhysicalDeviceFeatures Device::deviceFeatures{};

VkPhysicalDeviceVulkan13Features Device::features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
													 .synchronization2 = true,
													 .dynamicRendering = true };

VkPhysicalDeviceVulkan12Features Device::features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
													 .descriptorIndexing = true,
													 .bufferDeviceAddress = true };

Device::Device(const Instance& instance, Window& window, const std::vector<const char*>& extensions) : 
	_physDevice(VK_NULL_HANDLE),
	_logicalDevice(VK_NULL_HANDLE),
	_window(window),
	_instance(instance),
	_graphQueue(VK_NULL_HANDLE), 
	_presQueue(VK_NULL_HANDLE) {

	Logger* logger = Logger::get_logger();

	// Create the surface for the passed-in window. I don't necessarily like it being here, but we are keeping window creation separate from the engine, so this has to be here for now.
	this->_window.create_surface(_instance.handle());

	// Select the physical device to be used for rendering
	_physDevice = selectPhysicalDevice(_instance.handle(), _window.surface(), extensions);
	// Queue the physical device properties
	vkGetPhysicalDeviceProperties(_physDevice, &_physDeviceProperties);
	logger->log(_physDeviceProperties);

	// Find the queue families and assign their indices
	_indices = QueueFamily::findQueueFamilies(_physDevice, _window.surface());
	logger->log(_indices);
	std::set<uint32_t> uniqueQueueFamilies = { _indices.graphicsFamily.value(), _indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// Populate queue create infos
	float priority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &priority};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Chain the desired features together using pNext before feeding them into deviceCreateInfo
	VkPhysicalDeviceFeatures2 versionFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = deviceFeatures };
	features12.pNext = &features13;
	versionFeatures.pNext = &features12;

	// Set up the logical device
	VkDeviceCreateInfo deviceCreateInfo{
	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	.pNext = &versionFeatures,
	.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
	.pQueueCreateInfos = queueCreateInfos.data(),
	.enabledLayerCount = instance.validationLayersEnabled() ? static_cast<uint32_t>(Instance::validationLayers.size()) : 0,
	.ppEnabledLayerNames = instance.validationLayersEnabled() ? Instance::validationLayers.data() : VK_NULL_HANDLE,
	.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
	.ppEnabledExtensionNames = extensions.data()
	};

	if (vkCreateDevice(_physDevice, &deviceCreateInfo, nullptr, &_logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}
	logger->print("Vulkan device successfully created.");

	// Get handles for the graphics and present queues
	vkGetDeviceQueue(_logicalDevice, _indices.graphicsFamily.value(), 0, &_graphQueue);
	vkGetDeviceQueue(_logicalDevice, _indices.presentFamily.value(), 0, &_presQueue);
	
}

Device::~Device() {
	vkDestroyDevice(_logicalDevice, nullptr);
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*>& extensions) {
	Logger* logger = Logger::get_logger();

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	if (extensionCount < 1)
		return false;

	// Get supported extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
	logger->print_extensions("Available Device Extensions:", availableExtensions);
	logger->print_extensions("Required Device Extensions:", extensions);

	std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	if (requiredExtensions.empty()) {
		logger->print("Required extensions are supported by the physical device!");
		return true;
	}
	logger->print("Required extensions are NOT supported, selecting next device...");
	return false;
}

bool Device::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice, surface);
	
	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, Instance::deviceExtensions);

	bool swapchainAdequate = true; // false;
	if (extensionsSupported) {
		// SwapchainSupportDetails swapchainSupport = Swapchain::querySwapchainSupport(device, surface);
		// swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

VkPhysicalDevice Device::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions) {
	Logger* logger = Logger::get_logger();

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("Failed to find any GPU with Vulkan support!");

	logger->print("Selecting physical device...");

	// Get available devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	logger->print_devices(devices);

	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, surface)) {
			selectedDevice = device;
			logger->print("Selected a suitable physical device: ");
			break;
		}
	}

	if (selectedDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable physical device!");

	return selectedDevice;
}