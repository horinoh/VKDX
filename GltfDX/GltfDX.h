#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Gltf.h"

class GltfDX : public DXExt, public Gltf
{
private:
	using Super = DXExt;
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
		NodeMatrices.assign(Doc.nodes.size(), DirectX::XMMatrixIdentity());
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
	//virtual void PreProcess() override{}
	//virtual void PostProcess() override {}

	virtual void PushNode() override { Gltf::PushNode(); CurrentMatrix.push_back(CurrentMatrix.back()); }
	virtual void PopNode() override { Gltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t i) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) override;
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
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
			const D3D12_HEAP_PROPERTIES HeapProperties = {
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				D3D12_MEMORY_POOL_UNKNOWN,
				0,
				0
			};
			const DXGI_SAMPLE_DESC SD = { 1, 0 };
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { 1.0f, 0 } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
#if 1
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { 
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } 
		};
#endif
		DX::SerializeRootSignature(Blob, {
#if 1
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_VERTEX },
#endif
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}
	virtual void CreateCommandList() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			GraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
		}
		LOG_OK();
	}
	virtual void CreateConstantBuffer() override {
		//const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		PV.Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);

		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		PV.View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);

		ConstantBuffers.push_back(ConstantBuffer());
		CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(PV)));
	}
	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
		{
			DsvDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps[0])));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[0].Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[0].Resource->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
		{
			assert(!ImageResources.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
	}
	virtual void PopulateCommandList(const size_t i) override;

	std::vector<DirectX::XMMATRIX> CurrentMatrix = { DirectX::XMMatrixIdentity() };
	std::vector<DirectX::XMMATRIX> NodeMatrices;
	std::vector<DirectX::XMMATRIX> AnimNodeMatrices;

	FLOAT CurrentFrame = 0.0f;

	struct ProjView
	{
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX View;
	};
	using ProjView = struct ProjView;
	ProjView PV;
	std::vector<const DirectX::XMMATRIX*> InverseBindMatrices;
	std::vector<DirectX::XMMATRIX> JointMatrices;
	std::vector<float> MorphWeights;
};
#pragma endregion