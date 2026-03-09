#pragma once
#include <vulkan/vulkan.h>
#include<string>
#include<vector>

class Shader
{
public:
	Shader(const VkDevice& device, const std::string& filePath, VkShaderStageFlagBits stage);
	~Shader();
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	VkPipelineShaderStageCreateInfo getStageInfo();
	

private:
	VkPipelineShaderStageCreateInfo m_ShaderStageInfo;
	VkDevice m_device;
	VkShaderStageFlagBits m_stage;
	VkShaderModule m_Module;

	std::vector<char> readFile(const std::string& filePath);
	void createShaderModule(const std::vector<char>& code);
};
