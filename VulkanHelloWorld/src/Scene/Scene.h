#pragma once
#include "../Core/Devices.h"
#include "../Graphics/Model.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Material.h"
#include "../Graphics/Entity.h"
#include<vector>
#include<memory>
#include<string>
#include<unordered_map>

class Scene
{
public:
	Scene(Devices& device);
	~Scene();

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	std::shared_ptr<Model> loadModel(const std::string& path);
	std::shared_ptr<Texture> loadTexture(const std::string& path);
	std::shared_ptr<Texture> loadTexture(uint32_t color);

	void addMaterial(const std::shared_ptr<Material> mat);
	void addEntity(std::unique_ptr<Entity> entity);
	void addEntity(std::shared_ptr<Model> model, std::shared_ptr<Material> material);

	void drawMain(VkCommandBuffer cmd, uint32_t currentFrame);
	void drawforShadow(VkCommandBuffer cmd, VkPipelineLayout shadowPipelineLayout);

	std::vector<std::shared_ptr<Model>>& getModels(){ return m_models; }
	std::vector<std::shared_ptr<Texture>>& getTextures() { return m_textures; }
	std::vector<std::shared_ptr<Material>>& getMaterials() { return m_materials; }

private:
	Devices& m_device;

	std::vector<std::shared_ptr<Model>> m_models;
	std::vector<std::shared_ptr<Texture>> m_textures;
	std::vector<std::shared_ptr<Material>> m_materials;

	std::unordered_map<std::string, std::shared_ptr<Model>> m_modelCache;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;

	std::vector<std::unique_ptr<Entity>> m_entities;
};
