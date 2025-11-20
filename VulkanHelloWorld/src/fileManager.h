#pragma once
#include<vector>
#include<string>

class fileManager
{
public:
	static std::vector<char> readFile(const std::string& filePath);
};

