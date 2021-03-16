#pragma once

//!< USE_WINRT が定義されない場合は WRL が使用される、WINRT では 要C++17以降 (If USE_WINRT is not defined, WRL will be used, WINRT require C++17 or later)
#define USE_WINRT
#ifdef USE_WINRT
#include <winrt/base.h>
#define COM_PTR winrt::com_ptr
#define COM_PTR_GET(_x) _x.get()
#define COM_PTR_PUT(_x) _x.put()
#define COM_PTR_PUTVOID(_x) _x.put_void()
#define COM_PTR_UUIDOF_PUTVOID(_x) __uuidof(_x), COM_PTR_PUTVOID(_x)
#define COM_PTR_RESET(_x) _x = nullptr
#define COM_PTR_AS(_x, _y) winrt::copy_to_abi(_x, *_y.put_void());
#define COM_PTR_COPY(_x, _y) _x.copy_from(COM_PTR_GET(_y))
#else
#include <wrl.h>
#define COM_PTR Microsoft::WRL::ComPtr
#define COM_PTR_GET(_x) _x.Get()
#define COM_PTR_PUT(_x) _x.GetAddressOf()
#define COM_PTR_PUTVOID(_x) _x.GetAddressOf()
#define COM_PTR_UUIDOF_PUTVOID(_x) IID_PPV_ARGS(COM_PTR_PUTVOID(_x))
#define COM_PTR_RESET(_x) _x.Reset()
#define COM_PTR_AS(_x, _y) VERIFY_SUCCEEDED(_x.As(&_y))
#define COM_PTR_COPY(_x, _y) (_x = _y)
#endif

//!< ソフトウエアラスタライザ (Software rasterizer)
//#define USE_WARP

#define USE_STATIC_SAMPLER //!< [ TextureDX ] VK:USE_IMMUTABLE_SAMPLER相当

//!< HLSLからルートシグネチャを作成する (Create root signature from HLSL)
//#define USE_HLSL_ROOTSIGNATRUE //!< [ TriangleDX ]

//#define USE_BUNDLE //!< [ ParametricSurfaceDX ] VK:USE_SECONDARY_COMMAND_BUFFER相当

//#define USE_ROOT_CONSTANTS //!< [ TriangleDX ] VK:USE_PUSH_CONSTANTS相当
//#define USE_GAMMA_CORRECTION

#define USE_SHADER_BLOB_PART //!< [ TriangleDX ]

#define USE_DXC

#include <initguid.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_6.h>
#include <DirectXMath.h>

#ifdef USE_DXC
#include <dxcapi.h>
//!< DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h のものをここへ移植 (Same as defined in DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h)
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8  | (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24)
#endif

#define SHADER_ROOT_ACCESS_DENY_ALL (D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_VS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_GS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))
#define SHADER_ROOT_ACCESS_DS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))

/**
@brief 32 bit カラー DirectX::PackedVector::XMCOLOR
@note ARGBレイアウト

XMCOLOR Color32;
XMVECTOR Color128;

@note 128 bit カラー → 32 bit カラー
DirectX::PackedVector::XMStoreColor(&Color32, Color128);

@note 32 bit カラー → 128 bit カラー
Color128 = DirectX::PackedVector::XMLoadColor(Color32);
*/
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

#include <comdef.h>

//!< _DEBUG であれば何もしなくても PIX 使用可能、Release で PIX を使用したいような場合は USE_PIX を定義する (In case want to use pix in Release build, define USE_PIX)
//!< ソリューション右クリック - ソリューションのNuGetパッケージの管理 - 参照タブ - WinPixEventRuntimeで検索 - プロジェクトを選択してPIXをインストールしておくこと
//#define USE_PIX
#include <pix3.h>
//!< プログラムからキャプチャを行いたい場合 (Capture in program code)
#if defined(_DEBUG) || defined(USE_PIX)
#include <DXProgrammableCapture.h>
#endif

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(hr) if(FAILED(hr)) { Log(data(DX::GetHRESULTString(hr))); DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + DX::GetHRESULTString(hr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(hr) if(FAILED(hr)) { Win::ShowMessageBox(nullptr, data(DX::GetHRESULTString(hr))); }
#endif

#include "Cmn.h"
#include "Win.h"

/**
リソースが作成された時 MakeResident() され、破棄された時 Evict() される。
アプリから明示的にこれを行いたい場合は以下のようにする
ID3D12Resource* Resource;
const std::vector<ID3D12Pageable*> Pageables = { Resource };
Device->MakeResident(static_cast<UINT>(size(Pageables)), data(Pageables));
Device->Evict(static_cast<UINT>(size(Pageables)), data(Pageables));
*/

/**
CommandList、CommandAllocator はスレッドセーフではないので各スレッド毎に持つ必要がある
CommandQueue はスレッドフリーで各スレッドから使用可能
*/

class DX : public Cmn, public Win
{
private:
	using Super = Win;

public:
	class ResourceBase
	{
	public:
		COM_PTR<ID3D12Resource> Resource;
		void Create(ID3D12Device* Device, const size_t Size, const D3D12_HEAP_TYPE HT, const void* Source = nullptr) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, HT, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
		}
	};
	class VertexBuffer : public ResourceBase 
	{
	private:
		using Super = ResourceBase;
	public:
		D3D12_VERTEX_BUFFER_VIEW View;
		void Create(ID3D12Device* Device, const size_t Size, const UINT Stride) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);
			View = D3D12_VERTEX_BUFFER_VIEW({ .BufferLocation = Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(Size), .StrideInBytes = Stride });
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const size_t Size, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyBufferRegion(GCL, Upload, COM_PTR_GET(Resource), Size, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		void ExecuteCopyCommand(ID3D12Device* Device, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const size_t Size, const void* Source) {
			COM_PTR<ID3D12Resource> Upload;
			DX::CreateBufferResource(COM_PTR_PUT(Upload), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Size, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fence);
		}
	};
	class IndexBuffer : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		D3D12_INDEX_BUFFER_VIEW View;
		void Create(ID3D12Device* Device, const size_t Size, const DXGI_FORMAT Format) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);
			View = D3D12_INDEX_BUFFER_VIEW({ .BufferLocation = Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(Size), .Format = Format });
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const size_t Size, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyBufferRegion(GCL, Upload, COM_PTR_GET(Resource), Size, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		void ExecuteCopyCommand(ID3D12Device* Device, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const size_t Size, const void* Source) {
			COM_PTR<ID3D12Resource> Upload;
			DX::CreateBufferResource(COM_PTR_PUT(Upload), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Size, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fence);
		}
	};
	class IndirectBuffer : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		COM_PTR<ID3D12CommandSignature> CommandSignature;
		void Create(ID3D12Device* Device, const D3D12_DRAW_INDEXED_ARGUMENTS& DIA) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, sizeof(DIA), D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);
			const std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED }), };
			const D3D12_COMMAND_SIGNATURE_DESC CSD = { .ByteStride = static_cast<UINT>(sizeof(DIA)), .NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs), .NodeMask = 0 };
			Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(CommandSignature));
		}
		void Create(ID3D12Device* Device, const D3D12_DRAW_ARGUMENTS& DA) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, sizeof(DA), D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);
			const std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW }), };
			const D3D12_COMMAND_SIGNATURE_DESC CSD = { .ByteStride = static_cast<UINT>(sizeof(DA)), .NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs), .NodeMask = 0 };
			Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(CommandSignature));
		}
		void Create(ID3D12Device* Device, const D3D12_DISPATCH_ARGUMENTS& DA) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, sizeof(DA), D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);
			const std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH }), };
			const D3D12_COMMAND_SIGNATURE_DESC CSD = { .ByteStride = static_cast<UINT>(sizeof(DA)), .NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs), .NodeMask = 0 };
			Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(CommandSignature));
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const size_t Size, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyBufferRegion(GCL, Upload, COM_PTR_GET(Resource), Size, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		void ExecuteCopyCommand(ID3D12Device* Device, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const size_t Size, const void* Source) {
			COM_PTR<ID3D12Resource> Upload;
			DX::CreateBufferResource(COM_PTR_PUT(Upload), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Size, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fence);
		}
	};
	class ConstantBuffer : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const size_t Size, const void* Source = nullptr) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, RoundUp256(Size), D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
		}
	};
	class ShaderResourceBuffer : public ResourceBase
	{
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC View;
		void Create(ID3D12Device* Device, const size_t Size, const void* Source, const size_t Stride) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
			View = D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = D3D12_BUFFER_SRV({.FirstElement = 0, .NumElements = static_cast<UINT>(Size / Stride), .StructureByteStride = static_cast<UINT>(Stride), .Flags = D3D12_BUFFER_SRV_FLAG_NONE })
			});
		}
	};
	class Texture : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const DXGI_FORMAT Format) {
			DX::CreateTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, Format, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
		}
	};
	class DepthTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		D3D12_DEPTH_STENCIL_VIEW_DESC DSV;
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const D3D12_CLEAR_VALUE& CV) {
			DX::CreateRenderTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, CV, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			DSV = DepthOrArraySize == 1 ? 
				D3D12_DEPTH_STENCIL_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D, .Flags = D3D12_DSV_FLAG_NONE, .Texture2D = D3D12_TEX2D_DSV({.MipSlice = 0 }) }) :
				D3D12_DEPTH_STENCIL_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY, .Flags = D3D12_DSV_FLAG_NONE, .Texture2DArray = D3D12_TEX2D_ARRAY_DSV({.MipSlice = 0, .FirstArraySlice = 0, .ArraySize = DepthOrArraySize }) });
		}
	};
	class RenderTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		D3D12_RENDER_TARGET_VIEW_DESC RTV;
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const D3D12_CLEAR_VALUE& CV) {
			DX::CreateRenderTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, CV, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
			RTV = DepthOrArraySize == 1 ?
				D3D12_RENDER_TARGET_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D, .Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 }) }) :
				D3D12_RENDER_TARGET_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY, .Texture2DArray = D3D12_TEX2D_ARRAY_RTV({.MipSlice = 0, .FirstArraySlice = 0, .ArraySize = DepthOrArraySize, .PlaneSlice = 0 }) });
			SRV = DepthOrArraySize == 1 ?
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = Resource->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = CV.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = Resource->GetDesc().MipLevels, .FirstArraySlice = 0, .ArraySize = DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) });
		}
	};

#pragma region RAYTRACING
	class AccelerationStructureBuffer : public ResourceBase
	{
	public:
		void Create(ID3D12Device* Device, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		}
	};
	class ScratchBuffer : public ResourceBase
	{
	public:
		void Create(ID3D12Device* Device, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}; 
#pragma endregion
#pragma region MESH_SHADER
	struct MESH_SHADER_PIPELINE_STATE_DESC
	{
		ID3D12RootSignature* pRootSignature;
		D3D12_SHADER_BYTECODE         AS;
		D3D12_SHADER_BYTECODE         MS;
		D3D12_SHADER_BYTECODE         PS;
		D3D12_BLEND_DESC              BlendState;
		UINT                          SampleMask;
		D3D12_RASTERIZER_DESC         RasterizerState;
		D3D12_DEPTH_STENCIL_DESC      DepthStencilState;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		UINT                          NumRenderTargets;
		DXGI_FORMAT                   RTVFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		DXGI_FORMAT                   DSVFormat;
		DXGI_SAMPLE_DESC              SampleDesc;
		UINT                          NodeMask;
		D3D12_CACHED_PIPELINE_STATE   CachedPSO;
		D3D12_PIPELINE_STATE_FLAGS    Flags;
	};
#pragma endregion

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnPreDestroy(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override { Super::OnDestroy(hWnd, hInstance); }

	static [[nodiscard]] std::string GetHRESULTString(const HRESULT Result) { return ToString(GetHRESULTWString(Result)); }
	static [[nodiscard]] std::wstring GetHRESULTWString(const HRESULT Result) { return std::wstring(_com_error(Result).ErrorMessage()); }
	static [[nodiscard]] const char* GetFormatChar(const DXGI_FORMAT Format);

	static [[nodiscard]] std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT3(data(lhs));
		const auto r = DirectX::XMFLOAT3(data(rhs));
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&l), DirectX::XMLoadFloat3(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2] };
	}
	static [[nodiscard]] std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT4(data(lhs));
		const auto r = DirectX::XMFLOAT4(data(rhs));
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&l), DirectX::XMLoadFloat4(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2], v.m128_f32[3] };
	}

protected:
	static void ResourceBarrier(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After) {
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Resource,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = Before, .StateAfter = After
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}

	static void CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const size_t Size, const D3D12_RESOURCE_FLAGS RF, const D3D12_HEAP_TYPE HT, const D3D12_RESOURCE_STATES RS, const void* Source = nullptr);
	static void CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const std::vector<D3D12_SUBRESOURCE_DATA>& SRDs, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizeInBytes, const UINT64 TotalBytes);
	static void CreateTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, DXGI_FORMAT Format, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS);
	static void CreateRenderTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, const D3D12_CLEAR_VALUE& CV, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS);

	virtual void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range = nullptr);
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData);
	static void CreateBufferResourceAndExecuteCopyCommand(ID3D12Resource** Resource, ID3D12Device* Device, const size_t Size, 
		ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* Queue, ID3D12Fence* Fence, const void* Source);

public:
	static void PopulateCommandList_CopyBufferRegion(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES RS);
	static void PopulateCommandList_CopyBufferRegion(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	static void PopulateCommandList_CopyTextureRegion(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	
	static void ExecuteAndWait(ID3D12CommandQueue* CQ, ID3D12CommandList* CL, ID3D12Fence* Fence);

#if defined(_DEBUG) || defined(USE_PIX)
	//!< #DX_TODO PIX 関連
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName(ID3D12DeviceChild * Resource, LPCWSTR Name) { Resource->SetName(Name); }
	static void SetName(ID3D12DeviceChild* Resource, const std::wstring_view Name) { Resource->SetName(data(Name));}
#endif

	virtual void CreateDevice(HWND hWnd);
	virtual void LogAdapter(IDXGIAdapter* Adapter);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void LogOutput(IDXGIOutput* Output);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel(ID3D12Device* Device);
	virtual void CheckMultiSample(const DXGI_FORMAT Format);

	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateCommandList();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void GetSwapChainResource();
	//virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void ResetSwapChainResource() {
		for (auto& i : SwapChainResources) {
			COM_PTR_RESET(i);
		}
		SwapChainResources.clear();
	}
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);

	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);

	virtual void LoadScene() {}

#pragma region RAYTRACING
	static bool HasRaytracingSupport(ID3D12Device* Device) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 FDO5;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, reinterpret_cast<void*>(&FDO5), sizeof(FDO5)));
		return D3D12_RAYTRACING_TIER_NOT_SUPPORTED != FDO5.RaytracingTier;
	}
	static void BuildAccelerationStructure(ID3D12Device* Device, const UINT64 SBSize, const D3D12_GPU_VIRTUAL_ADDRESS GVA, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence);
#pragma endregion
	virtual void CreateGeometry() {}
	
	virtual void CreateConstantBuffer() {}

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);

	template<typename T = D3D12_ROOT_PARAMETER> void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<T>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path);
	virtual void CreateRootSignature();

	virtual void CreateDescriptorHeap() {}

	virtual void CreateDescriptorView() {}

	virtual void ProcessShaderReflection(ID3DBlob* Blob);
	virtual void SetBlobPart(COM_PTR<ID3DBlob>& Blob);
	virtual void GetBlobPart(ID3DBlob* Blob);
	virtual void StripShader(COM_PTR<ID3DBlob>& Blob);
#include "DXPipelineLibrary.inl"
	virtual void CreatePipelineState() {}
	static void CreatePipelineState_(COM_PTR<ID3D12PipelineState>& PST, ID3D12Device* Device, ID3D12RootSignature* RS,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology,
		const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
		const D3D12_RASTERIZER_DESC& RD,
		const D3D12_DEPTH_STENCIL_DESC& DSD,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, 
		const std::vector<DXGI_FORMAT>& RtvFormats,
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);
	//virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateTexture1x1(const UINT32 Color, const D3D12_RESOURCE_STATES RS = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	virtual void CreateTextureArray1x1(const std::vector<UINT32>& Colors, const D3D12_RESOURCE_STATES RS = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	virtual void CreateStaticSampler() {}
	virtual void CreateSampler() {}

	virtual void PopulateCommandList([[maybe_unused]] const size_t i) {}

	virtual UINT GetCurrentBackBufferIndex() const { return SwapChain->GetCurrentBackBufferIndex(); }
	virtual void DrawFrame([[maybe_unused]] const UINT i) {}
	virtual void Draw();
	virtual void Dispatch();
	virtual void WaitForFence();
	static void WaitForFence(ID3D12CommandQueue* CQ, ID3D12Fence* Fence);
	virtual void Submit();
	virtual void Present();

protected:
#if defined(_DEBUG) || defined(USE_PIX)
	COM_PTR<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif

	COM_PTR<IDXGIFactory4> Factory;
	COM_PTR<IDXGIAdapter> Adapter;
	COM_PTR<IDXGIOutput> Output;

	COM_PTR<ID3D12Device> Device;
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
	COM_PTR<ID3D12CommandQueue> GraphicsCommandQueue;
	COM_PTR<ID3D12CommandQueue> ComputeCommandQueue;
	COM_PTR<ID3D12Fence> Fence;
	UINT64 FenceValue = 0;

	std::vector<COM_PTR<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<COM_PTR<ID3D12CommandAllocator>> BundleCommandAllocators;

	std::vector<COM_PTR<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> BundleGraphicsCommandLists;

	COM_PTR<IDXGISwapChain4> SwapChain;
	std::vector<COM_PTR<ID3D12Resource>> SwapChainResources;

	//COM_PTR<ID3D12Resource> UnorderedAccessTextureResource;
	std::vector<COM_PTR<ID3D12Resource>> ImageResources;
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap; //!< RTVだけど、現状スワップチェインだけは別扱いにしている #DX_TODO
	std::vector<COM_PTR<ID3D12DescriptorHeap>> SamplerDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> RtvDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> DsvDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> CbvSrvUavDescriptorHeaps;

	std::vector<COM_PTR<ID3D12RootSignature>> RootSignatures;

	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	std::vector<COM_PTR<ID3D12PipelineState>> PipelineStates;

	std::vector<VertexBuffer> VertexBuffers;
	std::vector<IndexBuffer> IndexBuffers;
	std::vector<IndirectBuffer> IndirectBuffers;
#pragma region RAYTRACING
	std::vector<AccelerationStructureBuffer> BLASs, TLASs;
#pragma endregion
	std::vector<ConstantBuffer> ConstantBuffers;
	//std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC> ConstantBufferViewDescs;

	using Image = struct Image { COM_PTR<ID3D12Resource> Resource; };
	std::vector<Image> Images; 
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> ShaderResourceViewDescs; 
	std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC> DepthStencilViewDescs;
	std::vector<D3D12_RENDER_TARGET_VIEW_DESC> RenderTargetViewDescs;
	std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> UnorderedAccessViewDescs;
	std::vector<Texture> Textures;
	std::vector<DepthTexture> DepthTextures;
	std::vector<RenderTexture> RenderTextures;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	std::array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> DescriptorHandleIndices{};

protected:
	const D3D12_SHADER_BYTECODE NullShaderBC = { .pShaderBytecode = nullptr, .BytecodeLength = 0 };
};

#ifdef DEBUG_STDOUT
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMVECTOR& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.m128_f32[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMMATRIX& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.r[i]; } rhs << std::endl; return rhs; }
#endif