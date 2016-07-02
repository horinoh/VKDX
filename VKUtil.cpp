#include "stdafx.h"

#include "VK.h"

void VK::CreateShader_VsPs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"VS.vert.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"FS.frag.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateShader_VsPsTesTcsGs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"VS.vert.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"FS.frag.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"TES.tese.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"TCS.tesc.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"GS.geom.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateShader_Cs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"CS.cpom.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateVertexInput_Position()
{
	VertexInputBindingDescriptions = {
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateVertexInput_PositionColor()
{
	VertexInputBindingDescriptions = {
		{ 0, sizeof(glm::vec3) + sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) }
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
#endif
}
