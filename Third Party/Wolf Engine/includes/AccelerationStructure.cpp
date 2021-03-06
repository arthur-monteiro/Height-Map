#include "AccelerationStructure.h"

Wolf::AccelerationStructure::AccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice,
	VkCommandPool commandPool, Queue graphicsQueue, std::vector<BottomLevelAccelerationStructure::GeometryInfo> geometryInfo)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	m_commandPool = commandPool;

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	// Bottom Level AS
	BottomLevelAccelerationStructure::BottomLevelAccelerationStructureCreateInfo bottomLevelAccelerationStructureCreateInfo;
	bottomLevelAccelerationStructureCreateInfo.geometryInfos = geometryInfo;

	m_bottomLevelAccelerationStructures.resize(1);
	m_bottomLevelAccelerationStructures[0] = std::make_unique<BottomLevelAccelerationStructure>(device, physicalDevice, commandPool, commandBuffer, bottomLevelAccelerationStructureCreateInfo);

	// Top Level AS
	TopLevelAccelerationStructure::Instance topLevelInstance;
	topLevelInstance.bottomLevelAS = m_bottomLevelAccelerationStructures[0]->getAccelerationStructure();
	topLevelInstance.transform = geometryInfo[0].transform;
	topLevelInstance.instanceID = 0;
	topLevelInstance.hitGroupIndex = 0;

	std::vector<TopLevelAccelerationStructure::Instance> topLevelInstances = { topLevelInstance };
	m_topLevelAccelerationStructure = std::make_unique<TopLevelAccelerationStructure>(device, physicalDevice, commandBuffer, topLevelInstances);

	endSingleTimeCommands(device, graphicsQueue, commandBuffer, commandPool);
}
