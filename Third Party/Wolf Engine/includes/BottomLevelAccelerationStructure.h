#pragma once

#include <glm/gtx/transform.hpp>

#include "Mesh.h"
#include "VulkanHelper.h"

namespace Wolf
{
	class BottomLevelAccelerationStructure
	{
	public:
		struct GeometryInfo
		{
			VertexBuffer vertexBuffer;
			VkDeviceSize vertexSize;
			VkFormat vertexFormat;

			VkBuffer transformBuffer;
			VkDeviceSize transformOffsetInBytes;

			glm::mat4 transform;
		};
		
		struct BottomLevelAccelerationStructureCreateInfo
		{
			std::vector<GeometryInfo> geometryInfos;
		};
		
		BottomLevelAccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkCommandBuffer commandBuffer,
			BottomLevelAccelerationStructureCreateInfo& bottomLevelAccelerationStructureCreateInfo);

		VkAccelerationStructureNV getAccelerationStructure() { return m_accelerationStructureData.structure; }

	private:
		void getAccelerationBufferSizes();
		void buildAccelerationStructureBuffers();

	private:
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkCommandPool m_commandPool;
		VkCommandBuffer m_commandBuffer;
		
		std::vector<VkGeometryNV> m_vertexBuffers;
		VkDeviceSize m_scratchSizeInBytes = 0;
		VkDeviceSize m_resultSizeInBytes = 0;

		struct AccelerationStructureData
		{
			VkBuffer scratchBuffer = VK_NULL_HANDLE;
			VkDeviceMemory scratchMem = VK_NULL_HANDLE;
			
			VkBuffer resultBuffer = VK_NULL_HANDLE;
			VkDeviceMemory resultMem = VK_NULL_HANDLE;
			
			VkAccelerationStructureNV structure = VK_NULL_HANDLE;
		};
		AccelerationStructureData m_accelerationStructureData;
	};
}