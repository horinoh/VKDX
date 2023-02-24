#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../GLTF.h"

class GltfBaseVK : public VKExtDepth
{
private:
	using Super = VKExtDepth;

public:
	VkPrimitiveTopology Topology;
	VkIndexType IndexType = VkIndexType::VK_INDEX_TYPE_UINT32;
	std::vector<uint16_t> Indices16;
	std::vector<uint32_t> Indices32;
	std::vector<glm::vec3> Vertices;
	//std::vector<glm::vec3> Normals;

#pragma region GLTF
	virtual void LoadGltf() = 0;
#pragma endregion

	virtual void CreateGeometry() override {
		LoadGltf();

		const auto& CB = CommandBuffers[0];
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();

		VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Vertices));
		VK::Scoped<StagingBuffer> Staging_Vertex(Device);
		Staging_Vertex.Create(Device, PDMP, TotalSizeOf(Vertices), data(Vertices));

		//VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Normals));
		//VK::Scoped<StagingBuffer> Staging_Normal(Device);
		//Staging_Normal.Create(Device, PDMP, TotalSizeOf(Normals), data(Normals));

		VK::Scoped<StagingBuffer> Staging_Index(Device);
		uint32_t IndicesCount = 0;
		size_t IndicesSize = 0;
		if (!empty(Indices16)) {
			IndexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Indices16));
			Staging_Index.Create(Device, PDMP, TotalSizeOf(Indices16), data(Indices16));
			IndicesCount = static_cast<uint32_t>(size(Indices16));
			IndicesSize = TotalSizeOf(Indices16);
			IndexType = VkIndexType::VK_INDEX_TYPE_UINT16;
		}
		if (!empty(Indices32)) {
			IndexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Indices32));
			Staging_Index.Create(Device, PDMP, TotalSizeOf(Indices32), data(Indices32));
			IndicesCount = static_cast<uint32_t>(size(Indices32));
			IndicesSize = TotalSizeOf(Indices32);
			IndexType = VkIndexType::VK_INDEX_TYPE_UINT32;
		}
		assert(IndicesCount && IndicesSize && "");

		const VkDrawIndexedIndirectCommand DIIC = { .indexCount = IndicesCount, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers[0].PopulateCopyCommand(CB, TotalSizeOf(Vertices), Staging_Vertex.Buffer);
			//VertexBuffers[1].PopulateCopyCommand(CB, TotalSizeOf(Normals), Staging_Normal.Buffer);
			IndexBuffers.back().PopulateCopyCommand(CB, IndicesSize, Staging_Index.Buffer);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreatePipeline() override {
		const std::array SMs = {
			//VK::CreateShaderModule(GetFilePath("_PN.vert.spv")),
			//VK::CreateShaderModule(GetFilePath("_PN.frag.spv")),
			VK::CreateShaderModule(GetFilePath("_P.vert.spv")),
			VK::CreateShaderModule(GetFilePath("_P.frag.spv")),
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
		VKExt::CreatePipeline_VsFs_Input(Topology, 0, PRSCI, VK_TRUE, VIBDs, VIADs, PSSCIs);

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
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
			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, IndexType);

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

#ifdef USE_GLTF_SDK
class GltfVK : public GltfBaseVK, public Gltf::SDK
{
private:
	using Super = Gltf::SDK;
#pragma region GLTF
public:
	virtual void Process() override {
		Super::Process();

		for (const auto& i : Document.meshes.Elements()) {
			for (const auto& j : i.primitives) {
				switch (j.mode)
				{
				case Microsoft::glTF::MeshMode::MESH_POINTS: 
					Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_LINES: 
					Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_LINE_LOOP: break;
				case Microsoft::glTF::MeshMode::MESH_LINE_STRIP: 
					Topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLES: 
					Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLE_STRIP: 
					Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLE_FAN:
					Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
					break;
				default: break;
				}

				//!< 最初のやつだけ
				if (empty(Indices16) && empty(Indices32)) {
					if (Document.accessors.Has(j.indicesAccessorId)) {
						const auto& Accessor = Document.accessors.Get(j.indicesAccessorId);
						switch (Accessor.componentType)
						{
						case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_SHORT:
							switch (Accessor.type)
							{
							case Microsoft::glTF::AccessorType::TYPE_SCALAR:
							{
								Indices16.resize(Accessor.count);
								std::ranges::copy(ResourceReader->ReadBinaryData<uint16_t>(Document, Accessor), std::begin(Indices16));
							}
							break;
							default: break;
							}
							break;
						case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_INT:
							switch (Accessor.type)
							{
							case Microsoft::glTF::AccessorType::TYPE_SCALAR:
							{
								Indices32.resize(Accessor.count);
								std::ranges::copy(ResourceReader->ReadBinaryData<uint32_t>(Document, Accessor), std::begin(Indices32));
							}
							break;
							default: break;
							}
							break;
						default: break;
						}
					}
				}

				std::string AccessorId;
				//!< 最初のやつだけ
				if (empty(Vertices)) {
					if (j.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_POSITION, AccessorId))
					{
						const auto& Accessor = Document.accessors.Get(AccessorId);
						Vertices.resize(Accessor.count);
						switch (Accessor.componentType)
						{
						case Microsoft::glTF::ComponentType::COMPONENT_FLOAT:
							switch (Accessor.type)
							{
							case Microsoft::glTF::AccessorType::TYPE_VEC3:
							{
								std::memcpy(data(Vertices), data(ResourceReader->ReadBinaryData<float>(Document, Accessor)), TotalSizeOf(Vertices));

								AdjustScale(Vertices, 1.0f);
							}
							break;
							default: break;
							}
							break;
						default: break;
						}
					}
				}

				//!< 最初のやつだけ
				//if (empty(Normals)) {
				//	if (j.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_NORMAL, AccessorId))
				//	{
				//		const auto& Accessor = Document.accessors.Get(AccessorId);
				//		Normals.resize(Accessor.count);
				//		switch (Accessor.componentType)
				//		{
				//		case Microsoft::glTF::ComponentType::COMPONENT_FLOAT:
				//			switch (Accessor.type)
				//			{
				//			case Microsoft::glTF::AccessorType::TYPE_VEC3:
				//			{
				//				std::memcpy(data(Normals), data(ResourceReader->ReadBinaryData<float>(Document, Accessor)), TotalSizeOf(Normals));
				//			}
				//			break;
				//			default: break;
				//			}
				//			break;
				//		default: break;
				//		}
				//	}
				//}
			}
		}
	}
	virtual void LoadGltf() override {
		//!< データが埋め込まれていない(別ファイルになっている)タイプの場合は、カレントパスを変更しておくと読み込んでくれる
		Pushd(); {
			{
				//Pushd(GLTF_SAMPLE_PATH / "Suzanne" / "glTF"); {
				//	Load("Suzanne.gltf");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "Duck" / "glTF-Embedded"); {
				//	Load("Duck.gltf");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "WaterBottle" / "glTF-Binary"); {
				//	Load("WaterBottle.glb");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "AnimatedTriangle" / "glTF-Embedded"); {
				//	Load("AnimatedTriangle.gltf");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "RiggedSimple" / "glTF-Embedded"); {
				//	Load("RiggedSimple.gltf");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "RiggedFigure" / "glTF-Embedded"); {
				//	Load("RiggedFigure.gltf");
				//} Popd();

				//Pushd(GLTF_SAMPLE_PATH / "SimpleSkin" / "glTF-Embedded"); {
				//	Load("SimpleSkin.gltf");
				//} Popd();
			}
			{
				//Load(GLTF_PATH / "bunny.gltf");
				Load(GLTF_PATH / "dragon.gltf");
			}
		} Popd();
	}
#pragma endregion
};
#endif

#ifdef USE_GLTF_TINY
class GltfVK : public GltfBaseVK, public Gltf::Tiny
{
private:
public:
#pragma region GLTF
	virtual void Load(const std::filesystem::path& Path) override {
		Gltf::Tiny::Load(Path);
	}
#pragma endregion
};
#endif

#ifdef USE_GLTF_FX
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

#pragma endregion //!< Code