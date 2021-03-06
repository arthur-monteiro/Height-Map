#pragma once

#include "WolfEngine.h"
#include "Blur.h"

namespace Wolf
{
	class SSAO
	{
	public:
		SSAO(Wolf::WolfInstance* engineInstance, Wolf::Scene* scene, int commandBufferID, VkExtent2D extent, glm::mat4 projection,
			Image* depth, Image* normal, float near, float far);

		Texture* getOutputTexture() { return m_blur->getTextureOutput(); }

		std::vector<int> getCommandBufferIDs() { return m_blur->getCommandBufferIDs(); }
		std::vector<std::pair<int, int>> getCommandBufferSynchronisation() { return m_blur->getCommandBufferSynchronisation(); }

	private:
		int m_computePassID;
		Texture* m_outputTexture;

		struct UBOData
		{
			glm::mat4 projection;
			glm::mat4 invProjection;
			glm::vec4 projParams;
			glm::vec4 power = glm::vec4(6.0f);
			std::array<glm::vec4, 16> samples;
			std::array<glm::vec4, 16> noise;
		};
		UBOData m_uboData;
		UniformBuffer* m_uniformBuffer;

		std::unique_ptr<Blur> m_blur;
	};
}
