#define STB_IMAGE_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <array>
#include <set>
#include <stb_image.h>
#include "Core/ValidationLayerAssist.h"
#include "SwapChain.h"
#include <chrono>
#include "Buffer.h"
#include "Vertex.h"
#include "Description.h"
#include "Core/Devices.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineBuilder.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Framebuffer.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
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
		initVulkan();
		mainLoop();
		cleanUp();
	}

private:

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

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	bool framebufferResized = false;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		windowExtent = { WIDTH, HEIGHT };
	}

	void initVulkan()
	{
		createRenderPass();
		Descriptor::createDescriptorSetLayout(m_device->getLogicalDevice(), descriptorSetLayout);
		createGraphicsPipeline();
		createSwapchainFrameBuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		VertexBuffer::createVertexBuffer(m_device->getCommandPool(), m_device->getGraphicsQueue(), m_device->getLogicalDevice(), m_device->getPhysicalDevice(), vertexBuffer, vertexBufferMemory);
		IndexBuffer::createIndexBuffer(m_device->getCommandPool(), m_device->getGraphicsQueue(), m_device->getLogicalDevice(), m_device->getPhysicalDevice(), indexBuffer, indexBufferMemory);
		Descriptor::createUniformBuffers(m_device->getLogicalDevice(), m_device->getPhysicalDevice(), m_swapChain->getSwapChainImages(), uniformBuffers, uniformBuffersMemory);
		Descriptor::createDescriptorPool(m_device->getLogicalDevice(), m_swapChain->getSwapChainImages().size(), descriptorPool);
		Descriptor::createDescriptorSets(m_device->getLogicalDevice(), descriptorSetLayout, descriptorPool, m_swapChain->getSwapChainImages().size(), uniformBuffers, descriptorSets, textureImageView, textureSampler);
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop()
	{
		auto lastTime = std::chrono::high_resolution_clock::now();
		int frameCount = 0;

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
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
		vkDestroyImageView(m_device->getLogicalDevice(), textureImageView, nullptr);
		vkDestroyImage(m_device->getLogicalDevice(), textureImage, nullptr);
		vkFreeMemory(m_device->getLogicalDevice(), textureImageMemory, nullptr);
		vkDestroyDescriptorPool(m_device->getLogicalDevice(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_device->getLogicalDevice(), descriptorSetLayout, nullptr);
		vkDestroyBuffer(m_device->getLogicalDevice(), indexBuffer, nullptr);
		vkFreeMemory(m_device->getLogicalDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(m_device->getLogicalDevice(), vertexBuffer, nullptr);
		vkFreeMemory(m_device->getLogicalDevice(), vertexBufferMemory, nullptr);

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

		Descriptor::updateUniformBuffer(m_device->getLogicalDevice(),uniformBuffersMemory, m_swapChain->getSwapChainExtent(),imageIndex);

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
		// 3. 配置子流程 (对应原来的 VkSubpassDescription 和 Reference)
		SubpassConfig subpass = {};
		// 我们刚才加的颜色附件索引是 0，告诉子流程往 0 号坑位画
		subpass.colorAttachmentIndices = { 0 };
		// depthAttachmentIndex 默认是 -1，说明目前不需要深度测试

		m_mainRenderPass->addSubpass(subpass);

		DependencyConfig dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

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
		layout.push<glm::vec2>();
		layout.push<glm::vec3>();
		layout.push<glm::vec2>();

		PipelineBuilder builder;
		builder.shaderStages.push_back(vertShader.getStageInfo());
		builder.shaderStages.push_back(fragShader.getStageInfo());
		builder.setVertexInput(layout.getBindingDescription(), layout.getAttributeDescriptions());
		builder.viewport = { 0.0f,0.0f,(float)m_swapChain->getSwapChainExtent().width ,(float)m_swapChain->getSwapChainExtent().height ,0.0f,1.0f };
		builder.scissor = { {0,0}, m_swapChain->getSwapChainExtent() };

		descriptorSetLayouts.clear();
		descriptorSetLayouts.push_back(descriptorSetLayout);
		m_pipelineLayout = std::make_unique<PipelineLayout>(m_device->getLogicalDevice(), descriptorSetLayouts);
		builder.setPipelineLayout(m_pipelineLayout->getHandle());
		
		VkPipeline rawPipeline = builder.build(m_device->getLogicalDevice(), m_mainRenderPass->getHandle());
		m_pipeline = std::make_unique<Pipeline>(m_device->getLogicalDevice(), rawPipeline);


		//动态状态配置，我们把视口和裁剪矩形设置为动态的
		//一旦设置为动态状态，就必须在命令缓冲录制时通过vkCmdSetViewport和vkCmdSetScissor来重新设置它们，否则会报错
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

	}

	void createSwapchainFrameBuffers()
	{
		const std::vector<VkImageView>& swapchainImageViews = m_swapChain->getSwapChainImageViews();
		m_framebuffers.clear();
		m_framebuffers.reserve(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			std::vector<VkImageView> attachs = { swapchainImageViews[i] };

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
			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			//开始录制
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

			std::vector<VkBuffer> vertexbuffers = { vertexBuffer };
			std::vector<VkDeviceSize> offsets = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexbuffers.data(), offsets.data());
			
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout->getHandle(), 0, 1, &descriptorSets[i], 0, nullptr);
			
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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
		createSwapchainFrameBuffers();
		createCommandBuffers();
	}

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateImage(m_device->getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device->getLogicalDevice(), image, &memRequirements);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Buffer::findMemoryType(m_device->getPhysicalDevice(), memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(m_device->getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}
		vkBindImageMemory(m_device->getLogicalDevice(), image, imageMemory, 0);
	}

	void createTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("images/yoasobi.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		if(!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::createBuffer(m_device->getLogicalDevice(), m_device->getPhysicalDevice(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(m_device->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device->getLogicalDevice(), stagingBufferMemory);
		stbi_image_free(pixels);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Buffer::copyBufferToImage(m_device->getLogicalDevice(), m_device->getCommandPool(), m_device->getGraphicsQueue(), stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_device->getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->getLogicalDevice(), stagingBufferMemory, nullptr);
	}

	void createTextureImageView()
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		VkImageView imageView;
		if (vkCreateImageView(m_device->getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
		textureImageView = imageView;
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

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(m_device->getLogicalDevice(), m_device->getCommandPool());
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		//这里可以传递队列族所有权，但VK_QUEUE_FAMILY_IGNORED表示不做处理
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		CommandBuffer::endSingleTimeCommands(m_device->getLogicalDevice(), m_device->getCommandPool(), m_device->getGraphicsQueue(), commandBuffer);
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
