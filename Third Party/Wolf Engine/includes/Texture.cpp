#include "Texture.h"

Wolf::Texture::Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, Queue graphicsQueue)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	m_commandPool = commandPool;
	m_graphicsQueue = graphicsQueue;	
}

void Wolf::Texture::create(VkExtent3D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect)
{
	m_image = std::make_unique<Image>(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, extent, usage, format, sampleCount, aspect);
}

void Wolf::Texture::createFromImage(Image* image)
{
	m_imagePtr = image;
}

void Wolf::Texture::createFromPixels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	Queue graphicsQueue, VkExtent3D extent, VkFormat format, unsigned char* pixels)
{
	m_image = std::make_unique<Image>(device, physicalDevice, commandPool, graphicsQueue, extent, format, pixels);
}

bool Wolf::Texture::loadFromFile(std::string filename)
{
	m_image = std::make_unique<Image>(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, filename);

	return true;
}

void Wolf::Texture::createSampler(VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter)
{
	m_sampler = std::make_unique<Sampler>(m_device, addressMode, mipLevels, filter);
}

void Wolf::Texture::setImageLayout(VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	m_imagePtr == nullptr ?
		m_image->setImageLayout(newLayout, sourceStage, destinationStage) :
		m_imagePtr->setImageLayout(newLayout, sourceStage, destinationStage);
}

void Wolf::Texture::cleanup(VkDevice device)
{
	/*if (m_imagePtr == nullptr)
		m_image.cleanup(device);*/
}
