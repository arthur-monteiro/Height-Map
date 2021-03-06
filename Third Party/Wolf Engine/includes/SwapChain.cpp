#include <chrono>
#include "SwapChain.h"

Wolf::SwapChain::SwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	
	initialize(surface, window);
}

Wolf::SwapChain::~SwapChain()
{
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_imageAvailableSemaphore.cleanup(m_device);
}

void Wolf::SwapChain::cleanup()
{
	this->~SwapChain();
}

void Wolf::SwapChain::initialize(VkSurfaceKHR surface,
	GLFWwindow* window)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, surface);
	uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		throw std::runtime_error("Error : swapchain creation");

	// Get number of images
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);

	m_images.resize(imageCount);
	std::vector<VkImage> temporarySwapChainImages(imageCount);

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, temporarySwapChainImages.data());

	VkFormatProperties swapchainFormatProperties;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, surfaceFormat.format, &swapchainFormatProperties);

	if (!(swapchainFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		m_invertColors = true;
		//surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
	}

	for (size_t i(0); i < imageCount; ++i)
	{
		m_images[i] = std::make_unique<Image>(m_device, m_commandPool, m_graphicsQueue, temporarySwapChainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, extent);
		m_images[i]->setImageLayoutWithoutOperation(VK_IMAGE_LAYOUT_GENERAL);
	}

	m_imageAvailableSemaphore.initialize(m_device);
	m_imageAvailableSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

std::vector<Wolf::Image*> Wolf::SwapChain::getImages()
{
	std::vector<Image*> r;
	for (size_t i(0); i < m_images.size(); ++i)
		r.push_back(m_images[i].get());

	return r;
}


VkSurfaceFormatKHR Wolf::SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}

VkPresentModeKHR Wolf::SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	return bestMode;

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			bestMode = availablePresentMode;
	}

	return bestMode;
}

VkExtent2D Wolf::SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

uint32_t Wolf::SwapChain::getCurrentImage(VkDevice device)
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore.getSemaphore() , VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		//recreateSwapChain();
		//return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Error : can't acquire image");
	}

	return imageIndex;
}

void Wolf::SwapChain::present(Queue presentQueue, VkSemaphore waitSemaphore, uint32_t imageIndex)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	if (waitSemaphore)
	{
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;
	}
	else
	{
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
	}

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	/*if (m_maxFPS > 0)
	{
		long long microsecondOffset = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_lastFrameTimeCounter).count();
		if (microsecondOffset < (long long)((1.0 / m_maxFPS) * 1'000'000.0))
		{
			std::chrono::microseconds timespan((long long)((1.0 / m_maxFPS) * 1'000'000.0) - microsecondOffset - 1000);
			std::this_thread::sleep_for(timespan);
		}
	}
	 m_lastFrameTimeCounter = std::chrono::high_resolution_clock::now();*/

	presentQueue.mutex->lock();
	VkResult result = vkQueuePresentKHR(presentQueue.queue, &presentInfo);
	presentQueue.mutex->unlock();

	/*if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		recreateSwapChain();
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Erreur : affichage de la swapchain");*/

	presentQueue.mutex->lock();
	vkQueueWaitIdle(presentQueue.queue);
	presentQueue.mutex->unlock();
}

void Wolf::SwapChain::recreate(VkSurfaceKHR surface, GLFWwindow* window)
{
	vkDeviceWaitIdle(m_device);

	cleanup();
	m_images.clear();

	initialize(surface, window);
}
