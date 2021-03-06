#pragma once

#include "Image.h"
#include "Sampler.h"
#include "VulkanElement.h"

namespace Wolf
{
	class Texture : public VulkanElement
	{
	public:
		Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, Queue graphicsQueue);

		void create(VkExtent3D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect);
		void createFromImage(Image* image);
		void createFromPixels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, Queue graphicsQueue, VkExtent3D extent, VkFormat format, unsigned char* pixels);
		bool loadFromFile(std::string filename);
		void createSampler(VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter);

		void setImageLayout(VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

		void cleanup(VkDevice device);

		// Getters
	public:
		Image* getImage() { return m_imagePtr == nullptr ? m_image.get() : m_imagePtr; }
		VkImageView getImageView() { return m_imagePtr == nullptr ? m_image->getImageView() : m_imagePtr->getImageView(); }
		VkImageLayout getImageLayout() { return m_imagePtr == nullptr ? m_image->getImageLayout() : m_imagePtr->getImageLayout(); }
		VkSampler getSampler() { return m_sampler->getSampler(); }
		uint32_t getMipLevels() { return m_imagePtr == nullptr ? m_image->getMipLevels() : m_imagePtr->getMipLevels(); }

	private:
		std::unique_ptr<Image> m_image;
		Image* m_imagePtr = nullptr;

		std::unique_ptr<Sampler> m_sampler;
	};
}

