#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Gltf.h"

class GltfVK : public VKExt, public Gltf
{
private:
	using Super = VKExt;
public:
	GltfVK() : Super() {}
	virtual ~GltfVK() {}

	static VkIndexType ToVKIndexType(const fx::gltf::Accessor::ComponentType CT) {
		switch (CT) {
		case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_INDEX_TYPE_UINT16;
		case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_INDEX_TYPE_UINT32;
		}
		DEBUG_BREAK();
		return VK_INDEX_TYPE_MAX_ENUM;
	}

	static VkPrimitiveTopology ToVKPrimitiveTopology(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
		case fx::gltf::Primitive::Mode::Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case fx::gltf::Primitive::Mode::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			//case fx::gltf::Primitive::Mode::LineLoop:
		case fx::gltf::Primitive::Mode::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case fx::gltf::Primitive::Mode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case fx::gltf::Primitive::Mode::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case fx::gltf::Primitive::Mode::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}
		DEBUG_BREAK();
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

	static VkFormat ToVKFormat(const fx::gltf::Accessor& Acc) {
		switch (Acc.type) {
		//case fx::gltf::Accessor::Type::None:
		case fx::gltf::Accessor::Type::Scalar:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec2:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec3:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8B8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8B8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16B16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16B16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32B32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32B32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec4:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8B8A8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8B8A8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16B16A16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16B16A16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32B32A32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		//case fx::gltf::Accessor::Type::Mat2:
		//case fx::gltf::Accessor::Type::Mat3:
		//case fx::gltf::Accessor::Type::Mat4:
		}
		DEBUG_BREAK();
		return VK_FORMAT_UNDEFINED;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Document& Doc) override {
		NodeMatrices.assign(Doc.nodes.size(), glm::identity<glm::mat4>());
		Gltf::Process(Doc);
#ifdef DEBUG_STDOUT
		if (NodeMatrices.size()) {
			std::cout << "NodeMatrices[" << NodeMatrices.size() << "]" << std::endl;
			for (auto i : NodeMatrices) {
				std::cout << i;
			}
		}
#endif
	}
	virtual void PreProcess() override;
	virtual void PostProcess() override;

	virtual void PushNode() override { Gltf::PushNode(); CurrentMatrix.push_back(CurrentMatrix.back()); }
	virtual void PopNode() override { Gltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t i) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) override;
	virtual void Process(const fx::gltf::Mesh& Msh) override;
	virtual void Process(const fx::gltf::Skin& Skn) override;

	virtual std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) override { return VK::Lerp(lhs, rhs, t); }
	virtual std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) override { return VK::Lerp(lhs, rhs, t); }

	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;

	virtual void UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimWeights(const float* Data, const uint32_t PrevIndex, const uint32_t NextIndex, const float t);

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
#if 0
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
#endif
			});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
#if 0
				DescriptorSetLayouts[0]
#endif
			}, {});
	}
	virtual void CreateDepthStencil() override { VK::CreateDepthStencil(DepthFormat, GetClientRectWidth(), GetClientRectHeight()); }
	virtual void CreateFramebuffer() override { 
		const auto RP = RenderPasses[0];
		const auto DIV = DepthStencilImageView;
		for (auto i : SwapchainImageViews) {
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
		RenderPasses.resize(1);
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthAttach = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses[0], {
				//!< アタッチメント
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				{
					0,
					DepthFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				},
			}, {
				//!< サブパス
				{
					0,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					0, nullptr,
					static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
					&DepthAttach,
					0, nullptr
				},
			}, {
				//!< サブパス依存
			});
	}
	virtual void AllocateCommandBuffer() override {
		assert(!CommandPools.empty() && "");
		const auto PrevCount = CommandBuffers.size();
		CommandBuffers.resize(PrevCount + SwapchainImages.size());
		const VkCommandBufferAllocateInfo CBAI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPools[0],
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(SwapchainImages.size())
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers[PrevCount]));
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

	std::vector<glm::mat4> CurrentMatrix = { glm::identity<glm::mat4>() };
	std::vector<glm::mat4> NodeMatrices;
	std::vector<glm::mat4> AnimNodeMatrices;

	float CurrentFrame = 0.0f;
	
	struct ProjView
	{
		glm::mat4 Projection;
		glm::mat4 View;
	};
	using ProjView = struct ProjView;
	ProjView PV;	
	std::vector<const glm::mat4*> InverseBindMatrices;
	std::vector<glm::mat4> JointMatrices;
	std::vector<float> MorphWeights;

	uint32_t HeapIndex;
	VkDeviceSize Offset;
	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
	};
};
#pragma endregion