#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

#if 1
#define USE_GLTF_TINY
#include "../GLTF.h"

class GltfVK : public VKExt, public Gltf::Tiny
{
private:
	using Super = VKExt;
	using SuperGltf = Gltf::Tiny;
public:
	GltfVK() : Super() {}
	virtual ~GltfVK() {}

	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> Normals;
#pragma region GLTF
	virtual void Process(const tinygltf::Primitive& Primitive) {
		SuperGltf::Process(Primitive);

		auto Max = glm::vec3((std::numeric_limits<float>::min)());
		auto Min = glm::vec3((std::numeric_limits<float>::max)());
		//!< バーテックス
		for (auto i : Primitive.attributes) {
			const auto Acc = Model.accessors[i.second];
			const auto BufferView = Model.bufferViews[Acc.bufferView];
			const auto Buffer = Model.buffers[BufferView.buffer];
			const auto Stride = Acc.ByteStride(BufferView);
			const auto Size = Acc.count * Stride;

			if (TINYGLTF_COMPONENT_TYPE_FLOAT == Acc.componentType) {
				const float* p = reinterpret_cast<const float*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
				if ("POSITION" == i.first) {
					Vertices.resize(Acc.count);
					std::memcpy(data(Vertices), p, Size);

					for (auto j : Vertices) {
						Max.x = std::max(Max.x, j.x);
						Max.y = std::max(Max.y, j.y);
						Max.z = std::max(Max.z, j.z);
						Min.x = std::min(Min.x, j.x);
						Min.y = std::min(Min.y, j.y);
						Min.z = std::min(Min.z, j.z);
					}
					const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
					std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const glm::vec3& rhs) { return rhs / Bound - glm::vec3(0.0f, (Max.y - Min.y) * 0.5f, Min.z) / Bound; });
				}
				if ("NORMAL" == i.first) {
					Normals.resize(Acc.count);
					std::memcpy(data(Normals), p, Size);
				}
			}
		}

		//!< インデックス
		{
			const auto Acc = Model.accessors[Primitive.indices];
			const auto BufferView = Model.bufferViews[Acc.bufferView];
			const auto Buffer = Model.buffers[BufferView.buffer];
			const auto Stride = Acc.ByteStride(BufferView);
			const auto Size = Acc.count * Stride;
			Indices.resize(Acc.count);
			switch (Acc.componentType) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			{
				const uint16_t* p = reinterpret_cast<const uint16_t*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
				std::memcpy(data(Indices), p, Size);
			}
			break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			{
				const uint32_t* p = reinterpret_cast<const uint32_t*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
				std::memcpy(data(Indices), p, Size);
			}
			break;
			}
		}	
	}
#pragma endregion

#ifdef USE_RENDERDOC
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< #TIPS　レイトレーシングやメッシュシェーダーと同時に "VK_LAYER_RENDERDOC_Capture" を使用した場合に vkCreateDevice() でコケていたので、用途によっては注意が必要
		Super::CreateInstance({ "VK_LAYER_RENDERDOC_Capture" }, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override {
		Super::CreateDevice(hWnd, hInstance, pNext, { VK_EXT_DEBUG_MARKER_EXTENSION_NAME });
	}
#endif
	virtual void CreateGeometry() override {
		std::wstring Path;
		if (FindDirectory("GLTF", Path)) {
			Load(ToString(Path) + "//bunny.gltf");
			//Load(ToString(Path) + "//dragon.gltf");
		}
		//Load(std::string("..//tinygltf//models//Cube//") + "Cube.gltf");

		const auto& CB = CommandBuffers[0];
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();

		VertexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Vertices));
		VK::Scoped<StagingBuffer> Staging_Vertex(Device);
		Staging_Vertex.Create(Device, PDMP, Sizeof(Vertices), data(Vertices));

		//VertexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Normals));
		//VK::Scoped<StagingBuffer> Staging_Normal(Device);
		//Staging_Normal.Create(Device, PDMP, Sizeof(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Indices));
		VK::Scoped<StagingBuffer> Staging_Index(Device);
		Staging_Index.Create(Device, PDMP, Sizeof(Indices), data(Indices));

		const VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(size(Indices)), .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers[0].PopulateCopyCommand(CB, Sizeof(Vertices), Staging_Vertex.Buffer);
			//VertexBuffers[1].PopulateCopyCommand(CB, Sizeof(Normals), Staging_Normal.Buffer);
			IndexBuffers.back().PopulateCopyCommand(CB, Sizeof(Indices), Staging_Index.Buffer);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Depth(); }
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv")))
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::vector VIBDs = {
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertices[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			//VkVertexInputBindingDescription({.binding = 1, .stride = sizeof(Normals[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		const std::vector VIADs = {
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
			//VkVertexInputAttributeDescription({.location = 1, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
		};

		constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_LINE,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, PRSCI, VK_TRUE, VIBDs, VIADs, PSSCIs);

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures.back().View });
		}
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];

#pragma region SECONDARY_COMMAND_BUFFER
		const auto SCB = SecondaryCommandBuffers[i];
		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.subpass = 0,
			.framebuffer = FB,
			.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
			.pipelineStatistics = 0,
		};
		const VkCommandBufferBeginInfo SCBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB, &SCBBI)); {
			vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			const std::array VBs = { VertexBuffers[0].Buffer };
			//const std::array NBs = { VertexBuffers[1].Buffer };
			const std::array Offsets = { VkDeviceSize(0) };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
			//vkCmdBindVertexBuffers(SCB, 1, static_cast<uint32_t>(size(NBs)), data(NBs), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
#pragma endregion

		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RP,
				.framebuffer = FB,
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array SCBs = { SCB };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#else

#define USE_GLTF_FX
#include "../GLTF.h"

class GltfVK : public VKExt, public Gltf::Fx
{
private:
	using Super = VKExt;
	using SuperGltf = Gltf::Fx;
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
		SuperGltf::Process(Doc);
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

	virtual void PushNode() override { SuperGltf::PushNode(); CurrentMatrix.emplace_back(CurrentMatrix.back()); }
	virtual void PopNode() override { SuperGltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t i) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(std::string_view Identifier, const fx::gltf::Accessor& Acc) override;
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
//		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
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
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
#if 1
		VKExt::CreateRenderPass_Depth();
#else
		const std::array ColorAttach = { VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
		const VkAttachmentReference DepthAttach = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses.emplace_back(), {
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
	
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
	};
	using Transform = struct Transform;
	Transform Tr;	
	std::vector<const glm::mat4*> InverseBindMatrices;
	std::vector<glm::mat4> JointMatrices;
	std::vector<float> MorphWeights;

	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
	};
};
#endif
#pragma endregion