#include "Shader.h"
#include<fstream>

Shader::Shader(const VkDevice& device, const std::string& filePath, VkShaderStageFlagBits stage) :
	m_device(device), m_stage(stage)
{
	std::vector<char> ShaderCode = readFile(filePath);
	createShaderModule(ShaderCode);
	m_ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderStageInfo.stage = stage;
	m_ShaderStageInfo.module = m_Module;
	m_ShaderStageInfo.pName = "main";
	m_ShaderStageInfo.pSpecializationInfo = nullptr;
}

VkPipelineShaderStageCreateInfo Shader::getStageInfo()
{
	return m_ShaderStageInfo;
}

std::vector<char> Shader::readFile(const std::string& filePath)
{
	//以二进制方式打开，并且把文件指针移动到文件末尾
	//这样就可以通过tellg直接获取文件大小
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filePath);
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	//重新把文件指针移动到文件开头，读取所有数据
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void Shader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	m_Module = shaderModule;
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_device, m_Module, nullptr);
}

