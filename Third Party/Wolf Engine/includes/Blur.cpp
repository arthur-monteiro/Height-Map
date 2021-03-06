#include "Blur.h"

Wolf::Blur::Blur(Wolf::WolfInstance* engineInstance, Wolf::Scene* scene, int commandBufferID, Image* inputImage, Image* depthImage)
{
	m_inputImage = inputImage;
	m_commandBufferID = commandBufferID;

	// Downscale
	m_downscaledTextures.resize(3);
	m_downscaleComputePasses.resize(3);
	m_downscaleCommandBufferIDs.resize(3);
	VkExtent2D extent = { inputImage->getExtent().width / 2, inputImage->getExtent().height / 2 };
	for(int i(0);  i < 3; ++i)
	{
		m_downscaledTextures[i] = engineInstance->createTexture();
		m_downscaledTextures[i]->create({ extent.width, extent.height, 1 },
			VK_IMAGE_USAGE_STORAGE_BIT, inputImage->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_downscaledTextures[i]->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Scene::CommandBufferCreateInfo commandBufferCreateInfo;
		commandBufferCreateInfo.finalPipelineStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		commandBufferCreateInfo.commandType = Scene::CommandType::COMPUTE;
		m_downscaleCommandBufferIDs[i] = scene->addCommandBuffer(commandBufferCreateInfo);

		Scene::ComputePassCreateInfo downscaleComputePassCreateInfo;
		downscaleComputePassCreateInfo.extent = extent;
		downscaleComputePassCreateInfo.dispatchGroups = { 16, 16, 1 };
		downscaleComputePassCreateInfo.computeShaderPath = "Shaders/Blur/downscale.spv";
		downscaleComputePassCreateInfo.commandBufferID = m_downscaleCommandBufferIDs[i];

		DescriptorSetGenerator descriptorSetGenerator;
		descriptorSetGenerator.addImages({ i == 0 ? inputImage : m_downscaledTextures[i - 1]->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
			0); // Input
		descriptorSetGenerator.addImages({ m_downscaledTextures[i]->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
			1); // Output

		downscaleComputePassCreateInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

		m_downscaleComputePasses[i] = scene->addComputePass(downscaleComputePassCreateInfo);

		extent.width /= 2;
		extent.height /= 2;
	}	

	// Blur
	{
		m_downscaledBlurredTexture = engineInstance->createTexture();
		m_downscaledBlurredTexture->create({ m_downscaledTextures.back()->getImage()->getExtent().width, m_downscaledTextures.back()->getImage()->getExtent().height, 1 },
			VK_IMAGE_USAGE_STORAGE_BIT, inputImage->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_downscaledBlurredTexture->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Scene::CommandBufferCreateInfo commandBufferCreateInfo;
		commandBufferCreateInfo.finalPipelineStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		commandBufferCreateInfo.commandType = Scene::CommandType::COMPUTE;
		m_horizontalBlurCommandBuffer = scene->addCommandBuffer(commandBufferCreateInfo);

		// Horizontal
		{
			Scene::ComputePassCreateInfo horizontalBlurComputePassCreateInfo;
			horizontalBlurComputePassCreateInfo.extent = { m_downscaledTextures.back()->getImage()->getExtent().width, m_downscaledTextures.back()->getImage()->getExtent().height };
			horizontalBlurComputePassCreateInfo.dispatchGroups = { 16, 16, 1 };
			horizontalBlurComputePassCreateInfo.computeShaderPath = "Shaders/Blur/horizontal.spv";
			horizontalBlurComputePassCreateInfo.commandBufferID = m_horizontalBlurCommandBuffer;

			DescriptorSetGenerator descriptorSetGenerator;
			descriptorSetGenerator.addImages({ m_downscaledTextures.back()->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
			descriptorSetGenerator.addImages({ m_downscaledBlurredTexture->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);

			horizontalBlurComputePassCreateInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

			m_horizontalBlurComputePass = scene->addComputePass(horizontalBlurComputePassCreateInfo);
		}

		// Vertical
		{
			m_downscaledBlurredTexture2 = engineInstance->createTexture();
			m_downscaledBlurredTexture2->create({ m_downscaledTextures.back()->getImage()->getExtent().width, m_downscaledTextures.back()->getImage()->getExtent().height, 1 },
				VK_IMAGE_USAGE_STORAGE_BIT, inputImage->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
			m_downscaledBlurredTexture2->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			m_verticalBlurCommandBuffer = scene->addCommandBuffer(commandBufferCreateInfo);

			Scene::ComputePassCreateInfo verticalBlurComputePassCreateInfo;
			verticalBlurComputePassCreateInfo.extent = { m_downscaledTextures.back()->getImage()->getExtent().width, m_downscaledTextures.back()->getImage()->getExtent().height };
			verticalBlurComputePassCreateInfo.dispatchGroups = { 16, 16, 1 };
			verticalBlurComputePassCreateInfo.computeShaderPath = "Shaders/Blur/vertical.spv";
			verticalBlurComputePassCreateInfo.commandBufferID = m_verticalBlurCommandBuffer;

			DescriptorSetGenerator descriptorSetGenerator;
			descriptorSetGenerator.addImages({ m_downscaledBlurredTexture->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
			descriptorSetGenerator.addImages({ m_downscaledBlurredTexture2->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);

			verticalBlurComputePassCreateInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

			m_verticalBlurComputePass = scene->addComputePass(verticalBlurComputePassCreateInfo);
		}
	}
}
