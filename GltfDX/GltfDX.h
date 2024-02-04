#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../GLTF.h"

#include "WICTextureLoader.h"

class GltfBaseDX : public DXExtDepth
{
private:
	using Super = DXExtDepth;

public:
	D3D12_PRIMITIVE_TOPOLOGY Topology;
	std::vector<UINT16> Indices16;
	std::vector<UINT32> Indices32;
	std::vector<DirectX::XMFLOAT3> Vertices;
	//std::vector<DirectX::XMFLOAT3> Normals;

#pragma region GLTF
	virtual void LoadGltf() = 0;
#pragma endregion

	virtual void CreateGeometry() override {
		LoadGltf();
		
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CQ = COM_PTR_GET(GraphicsCommandQueue);

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), data(Vertices));

		//VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Normals), sizeof(Normals[0]));
		//UploadResource Upload_Normal;
		//Upload_Normal.Create(COM_PTR_GET(Device), TotalSizeOf(Normals), data(Normals));

		UploadResource Upload_Index;
		UINT32 IndicesCount = 0;
		size_t IndicesSize = 0;
		if (!empty(Indices16)) {
			IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Indices16), DXGI_FORMAT_R16_UINT);
			Upload_Index.Create(COM_PTR_GET(Device), TotalSizeOf(Indices16), data(Indices16));
			IndicesCount = static_cast<UINT32>(size(Indices16));
			IndicesSize = TotalSizeOf(Indices16);
		}
		if (!empty(Indices32)) {
			IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Indices32), DXGI_FORMAT_R32_UINT);
			Upload_Index.Create(COM_PTR_GET(Device), TotalSizeOf(Indices32), data(Indices32));
			IndicesCount = static_cast<UINT32>(size(Indices32));
			IndicesSize = TotalSizeOf(Indices32);
		}
		assert(IndicesCount && IndicesSize && "");

		const D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = IndicesCount, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			VertexBuffers[0].PopulateCopyCommand(CL, TotalSizeOf(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			//VertexBuffers[1].PopulateCopyCommand(CL, TotalSizeOf(Normals), COM_PTR_GET(Upload_Normal.Resource));
			IndexBuffers.back().PopulateCopyCommand(CL, IndicesSize, COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(CQ, CL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_DENY_ALL);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		PipelineStates.emplace_back();

		std::vector<COM_PTR<ID3DBlob>> SBs;
		//VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_PN.vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		//VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_PN.ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_P.vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_P.ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};
		const std::vector IEDs = {
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
			//D3D12_INPUT_ELEMENT_DESC({.SemanticName = "NORMAL", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 1, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};
		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_WIREFRAME,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		DXExt::CreatePipelineState_VsPs_Input(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, TRUE, IEDs, SBCs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#pragma region BUNDLE_COMMAND_LIST
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(Topology);

			//const std::array VBVs = { VertexBuffers[0].View, VertexBuffers[1].View };
			const std::array VBVs = { VertexBuffers[0].View };
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
#pragma endregion

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, PS));
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapchainBackBuffers[i].Resource);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const auto& HandleDSV = DsvDescs[0].second;

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(SwapchainBackBuffers[i].Handle, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				CL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { SwapchainBackBuffers[i].Handle };
				CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &HandleDSV[0]);

				CL->ExecuteBundle(BCL);
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};

#ifdef USE_GLTF_SDK
class GltfDX : public GltfBaseDX, public Gltf::SDK
{
private:
	using Super = Gltf::SDK;
#pragma region GLTF
public:
	virtual void Process(const Microsoft::glTF::Image& Image) override {
		Super::Process(Image);

		const auto Data = ResourceReader->ReadBinaryData(Document, Image);

		const std::array<uint8_t, 4> PNGHeader = { 0x89, 0x50, 0x4e, 0x47 };
		//const std::array<uint8_t, 2> JPGHeader = { 0xff, 0xd8 };
		//const std::array<uint8_t, 4> GIFGHeader = { 0x47, 0x49, 0x46, 0x38 };

		//!< ヘッダ識別 PNG の場合
		if (!std::memcmp(data(Data), data(PNGHeader), sizeof(PNGHeader))) {
			//COM_PTR<ID3D12Resource> Resource;
			//std::unique_ptr<uint8_t[]> DecodedData;
			//D3D12_SUBRESOURCE_DATA SubResource;
			//VERIFY_SUCCEEDED(DirectX::LoadWICTextureFromMemory(Device, data(Data), sizeof(Data), COM_PTR_PUT(Resource), DecodedData, SubResource, 0));
			//DecodedData.reset();
		}
	}
	virtual void Process() override {
		Super::Process();
		
		for (const auto& i : Document.meshes.Elements()) {
			for (const auto& j : i.primitives) {
				switch (j.mode)
				{
				case Microsoft::glTF::MeshMode::MESH_POINTS: 
					Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_LINES: 
					Topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_LINE_LOOP: break;
				case Microsoft::glTF::MeshMode::MESH_LINE_STRIP: 
					Topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLES: 
					Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLE_STRIP: 
					Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
					break;
				case Microsoft::glTF::MeshMode::MESH_TRIANGLE_FAN: break;
				default: break;
				}

				//!< 最初のやつだけ
				if (empty(Indices16)&& empty(Indices32)) {
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
				//Load(GLTF_PATH / "Box.glb");
				//Load(GLTF_PATH / "Sphere.glb");
			}
		} Popd();
	}
#pragma endregion
};
#endif

#ifdef USE_GLTF_TINY
class GltfDX : public GltfBaseDX, public Gltf::Tiny
{
private:
public:
#pragma region GLTF
	virtual void LoadGltf() override {
		std::filesystem::path Path;
		if (FindDirectory(GLTF_DIR, Path)) {
			Load(Path / TEXT("bunny.gltf"));
			//Load(Path) / TEXT("dragon.gltf"));
		}
	}

	virtual void Process(const tinygltf::Primitive& Primitive) {
		SuperGltf::Process(Primitive);

		auto Max = (std::numeric_limits<DirectX::XMFLOAT3>::min)();
		auto Min = (std::numeric_limits<DirectX::XMFLOAT3>::max)();
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
						Min = DX::Min(Min, j);
						Max = DX::Max(Max, j);
					}
					const auto Bound = (std::max)((std::max)(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
					std::ranges::transform(Vertices, std::begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });
				}
				if ("NORMAL" == i.first) {
					Normals.resize(Acc.count);
					std::memcpy(data(Normals), p, Size);
				}
			}
		}
		if (empty(Normals)) {
			Normals.assign(size(Vertices), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
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
};
#endif

#ifdef USE_GLTF_FX
class GltfDX : public DXExt, public Gltf::Fx
{
private:
	using Super = DXExt;
	using SuperGltf = Gltf::Fx;
public:
	GltfDX() : Super() {}
	virtual ~GltfDX() {}

	static DXGI_FORMAT ToDXFormat(const fx::gltf::Accessor::ComponentType CT) {
		switch (CT) {
			using enum fx::gltf::Accessor::ComponentType;
		case UnsignedShort: return DXGI_FORMAT_R16_UINT;
		case UnsignedInt: return DXGI_FORMAT_R32_UINT;
		}
		DEBUG_BREAK();
		return DXGI_FORMAT_UNKNOWN;
	}
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToDXPrimitiveTopologyType(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
			using enum fx::gltf::Primitive::Mode;
		case Points: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case Lines: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case LineLoop:
		case LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case Triangles:
		case TriangleStrip:
		case TriangleFan: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
		DEBUG_BREAK();
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
	static D3D12_PRIMITIVE_TOPOLOGY ToDXPrimitiveTopology(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
			using enum fx::gltf::Primitive::Mode;
		case Points: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case Lines: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		//case LineLoop:
		case LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case Triangles: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		//case TriangleFan:
		}
		DEBUG_BREAK();
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
	static DXGI_FORMAT ToDXFormat(const fx::gltf::Accessor& Acc) {
		switch (Acc.type) {
			using enum fx::gltf::Accessor::Type;
			//case None:
		case Scalar:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return DXGI_FORMAT_R8_SINT;
			case UnsignedByte: return DXGI_FORMAT_R8_UINT;
			case Short: return DXGI_FORMAT_R16_SINT;
			case UnsignedShort: return DXGI_FORMAT_R16_UINT;
			case UnsignedInt: return DXGI_FORMAT_R32_UINT;
			case Float: return DXGI_FORMAT_R32_FLOAT;
			}
		case Vec2:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return DXGI_FORMAT_R8G8_SINT;
			case UnsignedByte: return DXGI_FORMAT_R8G8_UINT;
			case Short: return DXGI_FORMAT_R16G16_SINT;
			case UnsignedShort: return DXGI_FORMAT_R16G16_UINT;
			case UnsignedInt: return DXGI_FORMAT_R32G32_UINT;
			case Float: return DXGI_FORMAT_R32G32_FLOAT;
			}
		case Vec3:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			//case Byte: return DXGI_FORMAT_R8G8B8_SINT;
			//case UnsignedByte: return DXGI_FORMAT_R8G8B8_UINT;
			//case Short: return DXGI_FORMAT_R16G16B16_SINT;
			//case UnsignedShort: return DXGI_FORMAT_R16G16B16_UINT;
			case UnsignedInt: return DXGI_FORMAT_R32G32B32_UINT;
			case Float: return DXGI_FORMAT_R32G32B32_FLOAT;
			}
		case Vec4:
			switch (Acc.componentType) {
				using enum fx::gltf::Accessor::ComponentType;
				//case None:
			case Byte: return DXGI_FORMAT_R8G8B8A8_SINT;
			case UnsignedByte: return DXGI_FORMAT_R8G8B8A8_UINT;
			case Short: return DXGI_FORMAT_R16G16B16A16_SINT;
			case UnsignedShort: return DXGI_FORMAT_R16G16B16A16_UINT;
			case UnsignedInt: return DXGI_FORMAT_R32G32B32A32_UINT;
			case Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			//case Mat2:
			//case Mat3:
			//case Mat4:
		}
		DEBUG_BREAK();
		return DXGI_FORMAT_UNKNOWN;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Document& Doc) override {
		NodeMatrices.assign(size(Doc.nodes), DirectX::XMMatrixIdentity());
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
	//virtual void PreProcess() override{}
	//virtual void PostProcess() override {}

	virtual void PushNode() override { SuperGltf::PushNode(); CurrentMatrix.emplace_back(CurrentMatrix.back()); }
	virtual void PopNode() override { SuperGltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t i) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(std::string_view Identifier, const fx::gltf::Accessor& Acc) override;
	virtual void Process(const fx::gltf::Mesh& Msh) override;
	virtual void Process(const fx::gltf::Skin& Skn) override;

	virtual std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) override { return DX::Lerp(lhs, rhs, t); }
	virtual std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) override { return DX::Lerp(lhs, rhs, t); }

	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;

	virtual void UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimWeights(const float* Data, const uint32_t PrevIndex, const uint32_t NextIndex, const float t);

	virtual void CreateTexture() override {
		CreateTexture_Depth();
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
#if 1
		constexpr std::array DRs = { 
			D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
#endif
		DX::SerializeRootSignature(Blob, {
#if 1
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX }),
#endif
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_VS);
#endif
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}
	virtual void CreateCommandList() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			DirectCommandLists.emplace_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(DirectCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(DirectCommandLists.back())));
			VERIFY_SUCCEEDED(DirectCommandLists.back()->Close());
		}
		LOG_OK();
	}
	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		Tr.Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);

		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Tr.View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);

		ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
	}
	virtual void CreateDescriptor() override {
		{
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
		{
			DsvDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
		}

		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[0].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[0].Resource->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
		{
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures[0].Resource), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
	}
	virtual void PopulateCommandList(const size_t i) override;

	std::vector<DirectX::XMMATRIX> CurrentMatrix = { DirectX::XMMatrixIdentity() };
	std::vector<DirectX::XMMATRIX> NodeMatrices;
	std::vector<DirectX::XMMATRIX> AnimNodeMatrices;

	FLOAT CurrentFrame = 0.0f;

	struct Transform
	{
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX View;
	};
	using Transform = struct Transform;
	Transform Tr;
	std::vector<const DirectX::XMMATRIX*> InverseBindMatrices;
	std::vector<DirectX::XMMATRIX> JointMatrices;
	std::vector<float> MorphWeights;
};
#endif

#pragma endregion //!< Code