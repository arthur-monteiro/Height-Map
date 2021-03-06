#include "DepthPass.h"

Wolf::DepthPass::DepthPass(Wolf::WolfInstance* engineInstance, Wolf::Scene* scene, int commandBufferID, bool outputIsSwapChain, VkExtent2D extent, VkSampleCountFlagBits sampleCount,
	Model* model, glm::mat4 mvp, bool useAsStorage, bool useAsSampled)
{
	m_engineInstance = engineInstance;
	m_scene = scene;

	// Render Pass Creation
	m_sampleCount = sampleCount;

	// Data
	m_mvp = glm::mat4(1.0f);
	m_uboMVP = engineInstance->createUniformBufferObject(&m_mvp, sizeof(glm::mat4));
	
	Scene::RenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.name = "Depth Pass";
	renderPassCreateInfo.commandBufferID = commandBufferID;
	renderPassCreateInfo.outputIsSwapChain = outputIsSwapChain; // should be equal to "no"

	VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (useAsStorage)
		usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	if(useAsSampled)
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	VkImageLayout layout;
	if (useAsStorage)
		layout = VK_IMAGE_LAYOUT_GENERAL;
	else
		layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	m_attachment = Attachment(extent, VK_FORMAT_D32_SFLOAT, m_sampleCount, layout, VK_ATTACHMENT_STORE_OP_STORE, usage);
	Scene::RenderPassOutput renderPassOutput;
	renderPassOutput.attachment = m_attachment;
	renderPassOutput.clearValue = { 1.0f };
	renderPassCreateInfo.outputs = { renderPassOutput };
	
	m_renderPassID = m_scene->addRenderPass(renderPassCreateInfo);

	// Renderer
	RendererCreateInfo rendererCreateInfo;

	ShaderCreateInfo vertexShaderCreateInfo{};
	vertexShaderCreateInfo.filename = "Shaders/depthPass/vert.spv";
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(vertexShaderCreateInfo);
	
	rendererCreateInfo.inputVerticesTemplate = InputVertexTemplate::FULL_3D_MATERIAL;
	rendererCreateInfo.instanceTemplate = InstanceTemplate::NO;
	rendererCreateInfo.renderPassID = m_renderPassID;
	rendererCreateInfo.pipelineCreateInfo.extent = extent;
	rendererCreateInfo.pipelineCreateInfo.alphaBlending = { false };

	DescriptorSetGenerator descriptorSetGenerator;
	descriptorSetGenerator.addUniformBuffer(m_uboMVP, VK_SHADER_STAGE_VERTEX_BIT, 0);

	rendererCreateInfo.descriptorLayouts = descriptorSetGenerator.getDescriptorLayouts();
	
	m_rendererID = scene->addRenderer(rendererCreateInfo);

	Renderer::AddMeshInfo addMeshInfo{};
	addMeshInfo.vertexBuffer = model->getVertexBuffers()[0];
	addMeshInfo.renderPassID = m_renderPassID;
	addMeshInfo.rendererID = m_rendererID;

	addMeshInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

	m_scene->addMesh(addMeshInfo);
}

void Wolf::DepthPass::update(glm::mat4 mvp)
{
	m_mvp = mvp;
	m_uboMVP->updateData(&m_mvp);
}
