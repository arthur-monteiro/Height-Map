#include "Scene.h"

using namespace Wolf;

::Scene::Scene(Wolf::WolfInstance* wolfInstance)
{
	m_window = wolfInstance->getWindowPtr();
	
	// Scene creation
	Wolf::Scene::SceneCreateInfo sceneCreateInfo;
	sceneCreateInfo.swapChainCommandType = Wolf::Scene::CommandType::GRAPHICS;

	m_scene = wolfInstance->createScene(sceneCreateInfo);

	// Render Pass Creation
	Wolf::Scene::RenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.commandBufferID = -1; // default command buffer
	renderPassCreateInfo.outputIsSwapChain = true;
	m_renderPassID = m_scene->addRenderPass(renderPassCreateInfo);

	// Heightmap creation (perlin noise)
	float totalWeight = 0;
	float weight = 1.0f;
	for (int div = 2; div < HEIGHMAP_RES; div *= 2)
	{
		for (int xFragment = 0; xFragment < div; ++xFragment)
		{
			for (int yFragment = 0; yFragment < div; ++yFragment)
			{
				float randNumber = rand(glm::vec2(xFragment, yFragment));
				float randNumberNextX = rand(glm::vec2(xFragment + 1, yFragment));
				float randNumberNextY = rand(glm::vec2(xFragment, yFragment + 1));
				float randNumberNextXY = rand(glm::vec2(xFragment + 1, yFragment + 1));

				for (int i = xFragment * (HEIGHMAP_RES / div); i < (xFragment + 1) * (HEIGHMAP_RES / div); ++i)
				{
					for (int j = yFragment * (HEIGHMAP_RES / div); j < (yFragment + 1) * (HEIGHMAP_RES / div); ++j)
					{
						float valueX1 = glm::mix(randNumber, randNumberNextX, ((float)i - ((float)xFragment * ((float)HEIGHMAP_RES / (float)div))) / ((float)HEIGHMAP_RES / (float)div));
						float valueX2 = glm::mix(randNumberNextY, randNumberNextXY, ((float)i - ((float)xFragment * ((float)HEIGHMAP_RES / (float)div))) / ((float)HEIGHMAP_RES / (float)div));
						float bilinearValue = glm::mix(valueX1, valueX2, ((float)j - (float)yFragment * (HEIGHMAP_RES / div)) / ((float)HEIGHMAP_RES / (float)div));
						m_heightMap[i][j] += bilinearValue * weight;
					}
				}
			}
		}
		totalWeight += weight;
		weight -= 0.1f;
		weight = std::max(weight, 0.1f);
	}

	for (int i = 0; i < HEIGHMAP_RES; ++i)
	{
		for (int j = 0; j < HEIGHMAP_RES; ++j)
		{
			m_heightMap[i][j] /= totalWeight;
		}
	}

	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;

	glm::vec3 topLeftPos(-100.0f, 0.0f, -100.0f);
	glm::vec3 tileSize(0.5f, 0.0f, 0.5f);
	float maxHeight = 50.0f;

	for (int i = 0; i < HEIGHMAP_RES - 1; ++i)
	{
		for (int j = 0; j < HEIGHMAP_RES - 1; ++j)
		{
			int firstIndice = vertices.size();

			vertices.push_back(glm::vec3(topLeftPos.x + i * tileSize.x, m_heightMap[i][j] * maxHeight, topLeftPos.z + j * tileSize.z));
			vertices.push_back(glm::vec3(topLeftPos.x + (i + 1) * tileSize.x, m_heightMap[i + 1][j] * maxHeight, topLeftPos.z + j * tileSize.z));
			vertices.push_back(glm::vec3(topLeftPos.x + i * tileSize.x, m_heightMap[i][j + 1] * maxHeight, topLeftPos.z + (j + 1) * tileSize.z));
			vertices.push_back(glm::vec3(topLeftPos.x + (i + 1) * tileSize.x, m_heightMap[i + 1][j + 1] * maxHeight, topLeftPos.z + (j + 1) * tileSize.z));

			indices.push_back(firstIndice);
			indices.push_back(firstIndice + 1);
			indices.push_back(firstIndice + 2);

			indices.push_back(firstIndice + 1);
			indices.push_back(firstIndice + 3);
			indices.push_back(firstIndice + 2);
		}
	}

	Model::ModelCreateInfo modelCreateInfo{};
	modelCreateInfo.inputVertexTemplate = InputVertexTemplate::NO;
	Model* model = wolfInstance->createModel<Vertex3D>(modelCreateInfo);
	model->addMeshFromVertices(vertices.data(), vertices.size(), sizeof(Vertex3D), indices); // data are pushed to GPU here

	RendererCreateInfo rendererCreateInfo;

	ShaderCreateInfo vertexShaderCreateInfo{};
	vertexShaderCreateInfo.filename = "Shaders/scene/vert.spv";
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(vertexShaderCreateInfo);

	ShaderCreateInfo fragmentShaderCreateInfo{};
	fragmentShaderCreateInfo.filename = "Shaders/scene/frag.spv";
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(fragmentShaderCreateInfo);

	rendererCreateInfo.inputVerticesTemplate = InputVertexTemplate::NO;
	rendererCreateInfo.instanceTemplate = InstanceTemplate::NO;
	rendererCreateInfo.pipelineCreateInfo.vertexInputBindingDescriptions = { Vertex3D::getBindingDescription(0) };
	rendererCreateInfo.pipelineCreateInfo.vertexInputAttributeDescriptions = { Vertex3D::getAttributeDescriptions(0) };
	rendererCreateInfo.renderPassID = m_renderPassID;

	rendererCreateInfo.pipelineCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;

	rendererCreateInfo.pipelineCreateInfo.alphaBlending = { true };

	DescriptorSetGenerator descriptorSetGenerator;

	m_ubData.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
	m_ubData.projection[1][1] *= -1;
	m_ubData.view = glm::lookAt(glm::vec3(-2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_ubData.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
	m_ub = wolfInstance->createUniformBufferObject(&m_ubData, sizeof(m_ubData));
	descriptorSetGenerator.addUniformBuffer(m_ub, VK_SHADER_STAGE_VERTEX_BIT, 0);

	rendererCreateInfo.descriptorLayouts = descriptorSetGenerator.getDescriptorLayouts();

	m_rendererID = m_scene->addRenderer(rendererCreateInfo);

	// Link the model to the renderer
	Renderer::AddMeshInfo addMeshInfo{};
	addMeshInfo.vertexBuffer = model->getVertexBuffers()[0];
	addMeshInfo.renderPassID = m_renderPassID;
	addMeshInfo.rendererID = m_rendererID;

	addMeshInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

	m_scene->addMesh(addMeshInfo);

	m_camera.initialize(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f,
		16.0f / 9.0f);

	// Record
	m_scene->record();
}

void ::Scene::update()
{
	m_camera.update(m_window);
	m_ubData.view = m_camera.getViewMatrix();

	m_ub->updateData(&m_ubData);
}