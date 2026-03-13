#define STB_IMAGE_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <array>
#include <set>
#include <stb_image.h>
#include "Core/ValidationLayerAssist.h"
#include <chrono>
#include "Buffer.h"
#include "Vertex.h"
#include "Description.h"
#include "Core/Devices.h"
#include "Renderer/Renderer.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineBuilder.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Camera.h"
#include "Graphics/Model.h"
#include "Graphics/Material.h"
#include "Graphics/Entity.h"

const uint32_t WIDTH = 2560;
const uint32_t HEIGHT = 1440;
const int MAX_FRAMES_IN_FLIGHT = 3;

class HelloTriangleApplication
{
public:

	GLFWwindow* window;
	void run()
	{
		initWindow();
		m_device = std::make_unique<Devices>(window);
		m_swapChain = std::make_unique<SwapChain>(*m_device, windowExtent);
		m_renderer = std::make_unique<Renderer>(*m_device, *m_swapChain, MAX_FRAMES_IN_FLIGHT);
		initVulkan();
		mainLoop();
		cleanUp();
	}

private:
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<Devices> m_device;
	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<PipelineLayout> m_pipelineLayout;
	std::unique_ptr<Pipeline> m_pipeline;
	std::unique_ptr<RenderPass> m_mainRenderPass;
	VkExtent2D windowExtent;

	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<std::unique_ptr<Framebuffer>> m_framebuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkSampler textureSampler;

	std::shared_ptr<Texture> m_yoasobiTex;
	std::shared_ptr<Texture> m_vikingRoomTex;
	std::unique_ptr<Texture> m_depthTex;
	std::shared_ptr<Model> m_vikingRoom;
	std::shared_ptr<Material> m_vikingRoomMat;
	std::unique_ptr<Entity> m_vikingEntity;
	std::unique_ptr<Entity> m_vikingEntity2;
	Camera m_camera{};
	float deltaTime;
	float lastFrame;

	// 追踪鼠标
	bool firstMouse = true;//防止鼠标刚移入窗口时计算错误

	//默认在窗口中心
	float lastX = WIDTH / 2.0f;
	float lastY = HEIGHT / 2.0f;

	bool framebufferResized = false;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetCursorPosCallback(window, mouseCallback); // 绑定鼠标回调
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 隐藏鼠标，沉浸式体验！
		windowExtent = { WIDTH, HEIGHT };
	}

	void initVulkan()
	{
		createRenderPass();
		Descriptor::createDescriptorSetLayout(m_device->getLogicalDevice(), descriptorSetLayout);
		createGraphicsPipeline();
		m_depthTex = Texture::createDepthTexture(*m_device, m_swapChain->getSwapChainExtent().width, m_swapChain->getSwapChainExtent().height);
		createSwapchainFrameBuffers();
		m_vikingRoomTex = Texture::loadFromFile(*m_device, "images/viking_room.png");
		m_yoasobiTex = Texture::loadFromFile(*m_device, "images/yoasobi.jpg");
		createTextureSampler();
		m_vikingRoom = std::make_unique<Model>(*m_device, "models/VikingRoom/viking_room.obj");
		Descriptor::createUniformBuffers(m_device->getLogicalDevice(), m_device->getPhysicalDevice(), m_swapChain->getSwapChainImages(), uniformBuffers, uniformBuffersMemory);
		
		
		m_vikingRoomMat = std::make_shared<Material>(*m_device, m_swapChain->getSwapChainImages().size(), descriptorSetLayout);
		
		m_vikingRoomMat->addTexture(1, m_vikingRoomTex);
		m_vikingRoomMat->addUniformBuffer(0, uniformBuffers,sizeof(UniformBufferObject));
		m_vikingRoomMat->build(textureSampler);

		m_vikingEntity = std::make_unique<Entity>(m_vikingRoom, m_vikingRoomMat);
		m_vikingEntity2 = std::make_unique<Entity>(m_vikingRoom, m_vikingRoomMat);
		m_vikingEntity2->setScale(glm::vec3{ 0.5f });
		m_vikingEntity2->setPosition(glm::vec3{ 2.0f,0.0f,0.0f });
		createCommandBuffers();
		createSyncObjects();
	}

	void processInput(GLFWwindow* window) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_camera.processKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_camera.processKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_camera.processKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_camera.processKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) m_camera.reset();
	}

	//C语言api绑定回调函数只能静态，因为非静态有this指针，它不认识这个
	static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->handleMouse(xposIn, yposIn);
	}

	void handleMouse(double xposIn, double yposIn) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		//float xoffset = xpos - lastX;
		float xoffset = lastX - xpos;
		float yoffset = lastY - ypos; // 注意这里是反的，因为屏幕Y坐标是从上往下的
		lastX = xpos;
		lastY = ypos;

		m_camera.processMouseMovement(xoffset, yoffset);
	}

	void mainLoop()
	{
		auto lastTime = std::chrono::high_resolution_clock::now();
		int frameCount = 0;
		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			glfwPollEvents();
			processInput(window);
			drawFrame();

			auto currentTime = std::chrono::high_resolution_clock::now();
			frameCount++;
			float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

			if (timeDiff >= 0.3f) {
				// 计算 FPS
				float fps = frameCount / timeDiff;
				int fpsinINT = static_cast<int>(fps);

				//std::cout << "FPS: " << fps << " (" << msPerFrame << " ms/frame)" << std::endl;
				std::string title = "Vulkan - FPS: " + std::to_string(fpsinINT);
				glfwSetWindowTitle(window, title.c_str());

				// 重置
				lastTime = currentTime;
				frameCount = 0;
			}
		}

		vkDeviceWaitIdle(m_device->getLogicalDevice());
	}

	static void framebufferResizeCallback(GLFWwindow* window,int width, int height)
	{
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void cleanUpSwapChain()
	{	
		vkFreeCommandBuffers(m_device->getLogicalDevice(), m_device->getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		m_framebuffers.clear();
		m_depthTex.reset();
		if (m_mainRenderPass) {
			m_mainRenderPass.reset();
		}
	}

	void cleanUp()
	{
		vkDeviceWaitIdle(m_device->getLogicalDevice());
		cleanUpSwapChain();
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vkDestroyBuffer(m_device->getLogicalDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(m_device->getLogicalDevice(), uniformBuffersMemory[i], nullptr);
		}

		vkDestroySampler(m_device->getLogicalDevice(), textureSampler, nullptr);
		m_vikingEntity2.reset();
		m_vikingEntity.reset();
		m_vikingRoomMat.reset();
		m_yoasobiTex.reset();
		m_vikingRoomTex.reset();
		vkDestroyDescriptorSetLayout(m_device->getLogicalDevice(), descriptorSetLayout, nullptr);
		m_vikingRoom.reset();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device->getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->getLogicalDevice(), inFlightFences[i], nullptr);
		}
		m_pipeline.reset();
		m_pipelineLayout.reset();
		m_swapChain.reset();
		m_device.reset();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void drawFrame()
	{
		/*
		* 有三步操作
		* 1.从交换链中获取下一张可用图像，立马返回可用的图像索引，同时cpu会立即执行第二步。但这个函数会等到图片可用后才把信号量imageAvailableSemaphore置为“已完成”。
		* 2.等到信号量imageAvailableSemaphore变“绿灯”时，执行command buffer里的命令，渲染图像，渲染完成后把信号量renderFinishedSemaphore置为“已完成”。
		* 3.等到信号量renderFinishedSemaphore变“绿灯”时呈现图像，把刚才渲染好的图像提交给交换链进行显示。
		*/

		vkWaitForFences(m_device->getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device->getLogicalDevice(), m_swapChain->getSwapChain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		Descriptor::updateUniformBuffer(m_device->getLogicalDevice(),uniformBuffersMemory, m_swapChain->getSwapChainExtent(),imageIndex, m_camera);

		vkResetFences(m_device->getLogicalDevice(), 1, &inFlightFences[currentFrame]);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::array<VkSemaphore,1> waitSemaphores = { imageAvailableSemaphores[currentFrame]};
		std::array<VkPipelineStageFlags,1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		std::array<VkSemaphore,1> signalSemaphores = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores.data();
		std::array<VkSwapchainKHR, 1> swapChains = { m_swapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains.data();
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}


		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void createRenderPass()
	{
		m_mainRenderPass = std::make_unique<RenderPass>(m_device->getLogicalDevice());
		AttachmentConfig colorAttachment = {};
		colorAttachment.format = m_swapChain->getSwapChainImageFormat();
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 渲染完用于显示
		m_mainRenderPass->addAttachment(colorAttachment);

		AttachmentConfig depthAttachment = {};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // 渲染完深度数据就可以丢弃了
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		m_mainRenderPass->addAttachment(depthAttachment);

		// 3. 配置子流程 (对应原来的 VkSubpassDescription 和 Reference)
		SubpassConfig subpass = {};
		// 我们刚才加的颜色附件索引是 0，告诉子流程往 0 号坑位画
		subpass.colorAttachmentIndices = { 0 };
		subpass.depthAttachmentIndex = 1;
		m_mainRenderPass->addSubpass(subpass);

		DependencyConfig dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		m_mainRenderPass->addDependency(dependency);
		m_mainRenderPass->create();
	}

	void createGraphicsPipeline()
	{
		/////////////////////////////////////////////////////////////////////////////////////
		////////////////////// 图形管线可编程阶段的配置(shaders) /////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////

		Shader vertShader(m_device->getLogicalDevice(), "shader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		Shader fragShader(m_device->getLogicalDevice(), "shader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShader.getStageInfo(), fragShader.getStageInfo() };
		
		/////////////////////////////////////////////////////////////////////////////////////
		////////////////////// 图形管线其他阶段的配置 (固定功能阶段) /////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////

		
		VertexLayout layout;
		layout.push<glm::vec3>();//位置
		layout.push<glm::vec3>();//颜色
		layout.push<glm::vec2>();//UV
		layout.push<glm::vec3>();//法线

		PipelineBuilder builder;
		builder.shaderStages.push_back(vertShader.getStageInfo());
		builder.shaderStages.push_back(fragShader.getStageInfo());
		builder.setVertexInput(layout.getBindingDescription(), layout.getAttributeDescriptions());
		builder.viewport = { 0.0f,0.0f,(float)m_swapChain->getSwapChainExtent().width ,(float)m_swapChain->getSwapChainExtent().height ,0.0f,1.0f };
		builder.scissor = { {0,0}, m_swapChain->getSwapChainExtent() };
		builder.enableDepthTest();

		descriptorSetLayouts.clear();
		descriptorSetLayouts.push_back(descriptorSetLayout);
		m_pipelineLayout = std::make_unique<PipelineLayout>(m_device->getLogicalDevice(), descriptorSetLayouts);
		builder.setPipelineLayout(m_pipelineLayout->getHandle());
		
		VkPipeline rawPipeline = builder.build(m_device->getLogicalDevice(), m_mainRenderPass->getHandle());
		m_pipeline = std::make_unique<Pipeline>(m_device->getLogicalDevice(), rawPipeline);

	}

	void createSwapchainFrameBuffers()
	{
		const std::vector<VkImageView>& swapchainImageViews = m_swapChain->getSwapChainImageViews();
		m_framebuffers.clear();
		m_framebuffers.reserve(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			std::vector<VkImageView> attachs = { swapchainImageViews[i], m_depthTex->getImageView()};

			m_framebuffers.push_back(std::make_unique<Framebuffer>(m_device->getLogicalDevice(),
				m_mainRenderPass->getHandle(), m_swapChain->getSwapChainExtent(), attachs));
		}
	}

	void createCommandBuffers()
	{
		commandBuffers.resize(m_framebuffers.size());
		
		//从commandPool里面申请内存给commandBuffers
		//有几个framebuffer就申请几个commandBuffer
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_device->getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_mainRenderPass->getHandle();;
			renderPassInfo.framebuffer = m_framebuffers[i]->getHandle();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f,0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			//开始录制
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

			// 配置并设置动态视口 (Viewport)
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
			viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f; // 深度范围：近平面
			viewport.maxDepth = 1.0f; // 深度范围：远平面
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			// 配置并设置动态裁剪矩形 (Scissor)
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_swapChain->getSwapChainExtent();
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			m_vikingEntity->draw(commandBuffers[i], m_pipelineLayout->getHandle(), i);
			m_vikingEntity2->draw(commandBuffers[i], m_pipelineLayout->getHandle(), i);
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
		}
		
	}

	void createSyncObjects()
	{
		size_t n = m_framebuffers.size();
		imageAvailableSemaphores.resize(n);
		renderFinishedSemaphores.resize(n);
		inFlightFences.resize(n);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}
		}

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateFence(m_device->getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create fences!");
			}
		}
	}

	void recreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_device->getLogicalDevice());
		m_swapChain.reset();
		cleanUpSwapChain();

		VkExtent2D newExtent = {width, height};
		m_swapChain = std::make_unique<SwapChain>(*m_device, newExtent);
		
		createRenderPass();
		createGraphicsPipeline();
		m_depthTex = Texture::createDepthTexture(*m_device, m_swapChain->getSwapChainExtent().width, m_swapChain->getSwapChainExtent().height);
		createSwapchainFrameBuffers();
		createCommandBuffers();
		std::cout << "Recreated Swapchain!" << std::endl;
	}

	void createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		if (vkCreateSampler(m_device->getLogicalDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

};


int main()
{
	HelloTriangleApplication app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
