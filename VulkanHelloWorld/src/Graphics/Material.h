#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Texture.h"
#include "../Core/Devices.h"

class Material
{
public:
	Material();
	~Material();

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;

private:
	Devices& m_device;
};


