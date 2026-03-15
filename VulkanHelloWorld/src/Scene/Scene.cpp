#include "Scene.h"


Scene::Scene(Devices& device) : m_device(device)
{

}

std::shared_ptr<Model> Scene::loadModel(const std::string& path)
{
	if (m_modelCache.contains(path))
	{
		return m_modelCache[path];
	}


	std::shared_ptr<Model> mod = std::make_shared<Model>(m_device, path);
	m_models.push_back(mod);
	m_modelCache[path] = mod;
	return mod;
}

std::shared_ptr<Texture> Scene::loadTexture(const std::string& path)
{
	if (m_textureCache.contains(path))
	{
		return m_textureCache[path];
	}

	std::shared_ptr<Texture> tex = Texture::loadFromFile(m_device, path);
	m_textures.push_back(tex);
	m_textureCache[path] = tex;
	return tex;
}

void Scene::addMaterial(const std::shared_ptr<Material> mat)
{
	m_materials.push_back(mat);
}

void Scene::addEntity(std::unique_ptr<Entity> entity)
{
	m_entities.push_back(std::move(entity));
}

void Scene::addEntity(std::shared_ptr<Model> model, std::shared_ptr<Material> material)
{
	m_entities.push_back(std::make_unique<Entity>(model, material));
}

void Scene::draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	for (auto& entity : m_entities)
	{
		entity->draw(cmd, pipelineLayout, currentFrame);
	}
}

Scene::~Scene()
{

}
