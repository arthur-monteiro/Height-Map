#pragma once

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

namespace Wolf
{
	class AccelerationStructure
	{
	public:		
		AccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, Queue graphicsQueue,
			std::vector<BottomLevelAccelerationStructure::GeometryInfo> geometryInfo);

		VkAccelerationStructureNV* getTopLevelAccelerationStructure() { return m_topLevelAccelerationStructure->getStructure(); }
		
	private:
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkCommandPool m_commandPool;
		
		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> m_bottomLevelAccelerationStructures;
		std::unique_ptr<TopLevelAccelerationStructure> m_topLevelAccelerationStructure;
	};

}
