#pragma once

#include "VulkanElement.h"

namespace Wolf
{
	class Buffer : public VulkanElement
	{
	public:
		Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDeviceSize size, VkBufferUsageFlags usage);
		~Buffer();

		VkBuffer getBuffer() { return m_buffer; }
		VkDeviceSize getSize() { return m_size; }

	private:
		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;
		VkDeviceSize m_size;
	};
}
