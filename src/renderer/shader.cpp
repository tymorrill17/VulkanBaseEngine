#include "renderer/shader.h"

void Shader::loadShaderModule(const std::string& filepath, const Device& device, VkShaderModule& outShaderModule) {
	// std::ios::ate -> puts stream curser at end
	// std::ios::binary -> opens file in binary mode
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::stringstream line;
		line << "ERROR: Shader file does not exist: " << filepath << std::endl;
		throw std::runtime_error(line.str());
	}
	// Since cursor is at the end, use it to find the size of the file, then copy the entire shader into a vector of uint32_t
	size_t filesize = (size_t)file.tellg(); // tellg() returns the position of the cursor
	std::vector<uint32_t> buffer(filesize / sizeof(uint32_t));
	file.seekg(0); // move cursor to beginning
	file.read((char*)buffer.data(), filesize); // load entire file into the buffer
	file.close();
	// Now we have the entire shader in the buffer and can load it to Vulkan
	VkShaderModuleCreateInfo createinfo;
	createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createinfo.pNext = nullptr;
	createinfo.codeSize = buffer.size() * sizeof(uint32_t); // codeSize has to be in bytes
	createinfo.pCode = buffer.data();
	createinfo.flags = 0;

	if (vkCreateShaderModule(device.device(), &createinfo, nullptr, &outShaderModule) != VK_SUCCESS) {
		std::stringstream line;
		line << "Error: vkCreateShaderModule() failed while creating " << filepath << std::endl;
		throw std::runtime_error(line.str());
	}
	std::cout << "Shader successfully loaded: " << filepath << std::endl;
}

VkPipelineShaderStageCreateInfo Shader::pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader) {
	VkPipelineShaderStageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.stage = stage,
		.module = shader,
		.pName = "main" // entry point of the shader program
	};
	return createInfo;
}