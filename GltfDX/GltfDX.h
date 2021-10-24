#pragma once

#include "resource.h"

#pragma region Code::
#include "../DXExt.h"

#if 1
#define USE_GLTF_TINY
#include "../GLTF.h"

class GltfDX : public DXExtDepth, public Gltf::Tiny
{
private:
	using Super = DXExtDepth;
	using SuperGltf = Gltf::Tiny;
public:
	std::vector<UINT32> Indices;
	std::vector<DirectX::XMFLOAT3> Vertices;
	std::vector<DirectX::XMFLOAT3> Normals;
#pragma region GLTF
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
					std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });
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

	GltfDX() : Super() {}
	virtual ~GltfDX() {}
	virtual void CreateGeometry() override {
		std::wstring Path;
		if (FindDirectory("GLTF", Path)) {
			Load(ToString(Path) + "//bunny.gltf");
			//Load(ToString(Path) + "//dragon.gltf");
		}
		//Load(std::string("..//tinygltf//models//Cube//") + "Cube.gltf");

		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CQ = COM_PTR_GET(GraphicsCommandQueue);

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Normals), sizeof(Normals[0]));
		UploadResource Upload_Normal;
		Upload_Normal.Create(COM_PTR_GET(Device), TotalSizeOf(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Indices), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_Index;
		Upload_Index.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), data(Indices));

		const D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = static_cast<UINT32>(size(Indices)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);

		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			VertexBuffers[0].PopulateCopyCommand(GCL, TotalSizeOf(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			VertexBuffers[1].PopulateCopyCommand(GCL, TotalSizeOf(Normals), COM_PTR_GET(Upload_Normal.Resource));
			IndexBuffers.back().PopulateCopyCommand(GCL, TotalSizeOf(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(GCL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(CQ, GCL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_DENY_ALL);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};
		const std::vector IEDs = {
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "NORMAL", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 1, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};
		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_WIREFRAME,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		DXExt::CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, TRUE, IEDs, SBCs);
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));

		Super::CreateDescriptor();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#pragma region BUNDLE_COMMAND_LIST
		const auto BGCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			const std::array VBVs = { VertexBuffers[0].View, VertexBuffers[1].View };
			BGCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BGCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());
#pragma endregion

		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				GCL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { SwapChainCPUHandles[i] };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);

				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#else
#define USE_GLTF_FX
#include "../GLTF.h"

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
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
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
#pragma endregion