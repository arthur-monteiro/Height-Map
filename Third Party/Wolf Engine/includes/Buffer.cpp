#include "Buffer.h"

Wolf::Buffer::Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDeviceSize size, VkBufferUsageFlags usage)
{
	m_device = device;
	m_size = size;

	createBuffer(device, physicalDevice, size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
}

Wolf::Buffer::~Buffer()
{
	vkDestroyBuffer(m_device, m_buffer, nullptr);
}
