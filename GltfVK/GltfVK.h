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
			using enum fx::gltf::Accessor::ComponentType;
		case UnsignedShort: return VK_INDEX_TYPE_UINT16;
		case UnsignedInt: return VK_INDEX_TYPE_UINT32;
		}
		DEBUG_BREAK();
		return VK_INDEX_TYPE_MAX_ENUM;
	}

	static VkPrimitiveTopology ToVKPrimitiveTopology(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
			using enum fx::gltf::Primitive::Mode;
		case Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		//case LineLoop:
		case LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}
		DEBUG_BREAK();
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

	static VkFormat ToVKFormat(const fx::gltf::Accessor& Acc) {
		switch (Acc.type) {
			using enum fx::gltf::Accessor::Type;
		//case None:
		case Scalar:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return VK_FORMAT_R8_SINT;
			case UnsignedByte: return VK_FORMAT_R8_UINT;
			case Short: return VK_FORMAT_R16_SINT;
			case UnsignedShort: return VK_FORMAT_R16_UINT;
			case UnsignedInt: return VK_FORMAT_R32_UINT;
			case Float: return VK_FORMAT_R32_SFLOAT;
			}
		case Vec2:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return VK_FORMAT_R8G8_SINT;
			case UnsignedByte: return VK_FORMAT_R8G8_UINT;
			case Short: return VK_FORMAT_R16G16_SINT;
			case UnsignedShort: return VK_FORMAT_R16G16_UINT;
			case UnsignedInt: return VK_FORMAT_R32G32_UINT;
			case Float: return VK_FORMAT_R32G32_SFLOAT;
			}
		case Vec3:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return VK_FORMAT_R8G8B8_SINT;
			case UnsignedByte: return VK_FORMAT_R8G8B8_UINT;
			case Short: return VK_FORMAT_R16G16B16_SINT;
			case UnsignedShort: return VK_FORMAT_R16G16B16_UINT;
			case UnsignedInt: return VK_FORMAT_R32G32B32_UINT;
			case Float: return VK_FORMAT_R32G32B32_SFLOAT;
			}
		case Vec4:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return VK_FORMAT_R8G8B8A8_SINT;
			case UnsignedByte: return VK_FORMAT_R8G8B8A8_UINT;
			case Short: return VK_FORMAT_R16G16B16A16_SINT;
			case UnsignedShort: return VK_FORMAT_R16G16B16A16_UINT;
			case UnsignedInt: return VK_FORMAT_R32G32B32A32_UINT;
			case Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		//case Mat2:
		//case Mat3:
		//case Mat4:
		}
		DEBUG_BREAK();
		return VK_FORMAT_UNDEFINED;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Document& Doc) override {
		NodeMatrices.assign(size(Doc.nodes), glm::identity<glm::mat4>());
		Gltf::Process(Doc);
#ifdef DEBUG_STDOUT
		if (size(NodeMatrices)) {
			std::cout << "NodeMatrices[" << size(NodeMatrices) << "]" << std::endl;
			for (auto i : NodeMatrices) {
				std::cout << i;
			}
		}
#endif
	}
	virtual void PreProcess() override;
	virtual void PostProcess() override;

	virtual void PushNode() override { Gltf::PushNode(); CurrentMatrix.emplace_back(CurrentMatrix.back()); }
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

//	virtual void CreateDescriptorSetLayout() override {
//		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
//		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
//#if 0
//				VkDescriptorSetLayoutBinding({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr })
//#endif
//			});
//	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
#if 0
			VkDescriptorSetLayoutBinding({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr })
#endif
		});

		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), {
#if 0
			DescriptorSetLayouts[0]
#endif
			}, {});
	}
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
	}
	virtual void CreateFramebuffer() override { 
		const auto RP = RenderPasses[0];
		const auto DIV = DepthTextures[0].View;
		for (auto i : SwapchainImageViews) {
			Framebuffers.emplace_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
#if 1
		VKExt::CreateRenderPass_Depth();
#else
		RenderPasses.emplace_back(VkRenderPass());
		const std::array ColorAttach = { VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
		const VkAttachmentReference DepthAttach = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses.back(), {
				//!< アタッチメント
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}),
				VkAttachmentDescription({
					.flags = 0,
					.format = DepthFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}),
			}, {
				//!< サブパス
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = &DepthAttach,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, {
				//!< サブパス依存
			});
#endif
	}
	virtual void AllocateCommandBuffer() override {
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));

		assert(!empty(CommandPools) && "");
		const auto PrevCount = size(CommandBuffers);
		CommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = SCCount
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

	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
	};
};
#pragma endregion