#include "fileManager.h"
#include<fstream>


std::vector<char> fileManager::readFile(const std::string& filePath)
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
