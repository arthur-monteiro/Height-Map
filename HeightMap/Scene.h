#pragma once

#include <WolfEngine.h>
#include <Template3D.h>

#include "Camera.h"

#define HEIGHMAP_RES 1024

class Scene
{
public:
	Scene(Wolf::WolfInstance* wolfInstance);

	void update();

	Wolf::Scene* getScene() const { return m_scene; }
	std::vector<int> getCommandBufferToSubmit() { return {}; }
	std::vector<std::pair<int, int>> getCommandBufferSynchronisation() { return {}; }

private:
	float rand(glm::vec2 co)
	{
		return glm::fract(glm::sin(glm::dot(co, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
	}

private:
	Camera m_camera;
	GLFWwindow* m_window;
	
	std::array<std::array<float, HEIGHMAP_RES>, HEIGHMAP_RES> m_heightMap;

	struct Vertex3D
	{
		glm::vec3 pos;

		static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = binding;
			bindingDescription.stride = sizeof(Vertex3D);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

			attributeDescriptions[0].binding = binding;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

			return attributeDescriptions;
		}

		bool operator==(const Vertex3D& other) const
		{
			return pos == other.pos;
		}
	};
	
	Wolf::Scene* m_scene = nullptr;
	int m_renderPassID = -1;
	int m_rendererID = -1;

	struct UniformBufferData
	{
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
	};
	UniformBufferData m_ubData;
	Wolf::UniformBuffer* m_ub;
};

