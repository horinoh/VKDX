#pragma once
 
//!< CCOM, WINRT, WRL が選択可能
#define USE_CCOMPTR
#define USE_WINRT

#ifdef USE_CCOMPTR
#include <atlbase.h>
#define COM_PTR CComPtr
#define COM_PTR_GET(_x) _x.p
#define COM_PTR_PUT(_x) &_x
#define COM_PTR_UUIDOF_PUTVOID(_x) IID_PPV_ARGS(&_x)
#define COM_PTR_RESET(_x) _x = nullptr
#define COM_PTR_AS(_x, _y) _y = _x
#define COM_PTR_COPY(_x, _y) _x = _y
#elif defined(USE_WINRT)
#include <winrt/base.h>
#define COM_PTR winrt::com_ptr
#define COM_PTR_GET(_x) _x.get()
#define COM_PTR_PUT(_x) _x.put()
#define COM_PTR_UUIDOF_PUTVOID(_x) __uuidof(_x), _x.put_void()
#define COM_PTR_RESET(_x) _x = nullptr
#define COM_PTR_AS(_x, _y) winrt::copy_to_abi(_x, *_y.put_void());
#define COM_PTR_COPY(_x, _y) _x.copy_from(COM_PTR_GET(_y))
#else
#include <wrl.h>
#define COM_PTR Microsoft::WRL::ComPtr
#define COM_PTR_GET(_x) _x.Get()
#define COM_PTR_PUT(_x) _x.GetAddressOf()
#define COM_PTR_UUIDOF_PUTVOID(_x) IID_PPV_ARGS(_x.GetAddressOf())
#define COM_PTR_RESET(_x) _x.Reset()
#define COM_PTR_AS(_x, _y) VERIFY_SUCCEEDED(_x.As(&_y))
#define COM_PTR_COPY(_x, _y) (_x = _y)
#endif

#include <initguid.h>
#include <d3d12.h>
#include <d3d12video.h>
#include <d3dcompiler.h>
#include <DXGI1_6.h>
#include <DirectXMath.h>

#ifndef VERIFY_SUCCEEDED
#ifdef _DEBUG
//#define VERIFY_SUCCEEDED(X) { const auto _HR = (X); if(FAILED(_HR)) { OutputDebugStringA(data(std::system_category().message(_HR) + "\n")); DEBUG_BREAK(); } }
#define VERIFY_SUCCEEDED(X) { const auto _HR = (X); if(FAILED(_HR)) { MessageBoxA(nullptr, data(std::system_category().message(_HR)), "", MB_OK); DEBUG_BREAK(); /*throw std::runtime_error("");*/ } }
#else
#define VERIFY_SUCCEEDED(X) (X) 
#endif
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

#ifdef USE_DXC
#include <dxcapi.h>
//!< DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h のものをここへ移植 (Same as defined in DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h)
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8  | (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24)
#endif

#define SHADER_ROOT_ACCESS_DENY_VS_HS_DS_GS_PS (D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_DENY_AS_MS (D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS)

//!< メッシュシェーダ系のフラグが含まれるとルートシグネチャ作成時にコケることがある
//#define SHADER_ROOT_ACCESS_DENY_ALL (SHADER_ROOT_ACCESS_DENY_VS_HS_DS_GS_PS)
#define SHADER_ROOT_ACCESS_DENY_ALL (SHADER_ROOT_ACCESS_DENY_VS_HS_DS_GS_PS | SHADER_ROOT_ACCESS_DENY_AS_MS)

#define SHADER_ROOT_ACCESS_VS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_GS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_VS_GS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS))
#define SHADER_ROOT_ACCESS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))
#define SHADER_ROOT_ACCESS_DS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))
#define SHADER_ROOT_ACCESS_MS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS)

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
#include <system_error>

//!< ソリューション右クリック - ソリューションのNuGetパッケージの管理 - 参照タブ - WinPixEventRuntime で検索 - プロジェクトを選択して PIX をインストールしておくこと
//#define USE_PIX
//!< プログラムからキャプチャを行いたい場合 (Capture in program code)
#ifdef USE_PIX
#include <pix3.h>
#include <DXProgrammableCapture.h>
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
	static constexpr DirectX::XMFLOAT3 Min(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3((std::min)(lhs.x, rhs.x), (std::min)(lhs.y, rhs.y), (std::min)(lhs.z, rhs.z)); }
	static constexpr DirectX::XMFLOAT3 Max(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3((std::max)(lhs.x, rhs.x), (std::max)(lhs.y, rhs.y), (std::max)(lhs.z, rhs.z)); }

	class ResourceBase
	{
	public:
		COM_PTR<ID3D12Resource> Resource;
		ResourceBase& Create(ID3D12Device* Device, const size_t Size, const D3D12_HEAP_TYPE HT, const void* Source = nullptr) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, HT, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
			return *this;
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const size_t Size, ID3D12Resource* Upload, const D3D12_RESOURCE_STATES RS = D3D12_RESOURCE_STATE_GENERIC_READ) {
			GCL->CopyBufferRegion(COM_PTR_GET(Resource), 0, Upload, 0, Size);
			{
				const std::array RBs = {
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
							.pResource = COM_PTR_GET(Resource),
							.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
							.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = RS
						})
					})
				};
				GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}
		}
		void ExecuteCopyCommand(ID3D12Device* Device, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const size_t Size, const void* Source, const D3D12_RESOURCE_STATES RS = D3D12_RESOURCE_STATE_GENERIC_READ) {
			UploadResource Upload;
			Upload.Create(Device, Size, Source);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Size, COM_PTR_GET(Upload.Resource), RS);
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fence);
		}
	};
	class DefaultResource : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		DefaultResource& Create(ID3D12Device* Device, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
			return *this;
		}
	};
	class UploadResource : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		UploadResource& Create(ID3D12Device* Device, const size_t Size, const void* Source = nullptr) { 
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source); 
			return *this;
		}
	};
	class VertexBuffer : public DefaultResource
	{
	private:
		using Super = DefaultResource;
	public:
		D3D12_VERTEX_BUFFER_VIEW View;
		VertexBuffer& Create(ID3D12Device* Device, const size_t Size, const UINT Stride) {
			Super::Create(Device, Size);
			View = D3D12_VERTEX_BUFFER_VIEW({ .BufferLocation = Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(Size), .StrideInBytes = Stride });
			return *this;
		}
	};
	class IndexBuffer : public DefaultResource
	{
	private:
		using Super = DefaultResource;
	public:
		D3D12_INDEX_BUFFER_VIEW View;
		IndexBuffer& Create(ID3D12Device* Device, const size_t Size, const DXGI_FORMAT Format) {
			Super::Create(Device, Size);
			View = D3D12_INDEX_BUFFER_VIEW({ .BufferLocation = Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(Size), .Format = Format });
			return *this;
		}
	};
	class IndirectBuffer : public DefaultResource
	{
	private:
		using Super = DefaultResource;
	protected:
		IndirectBuffer& Create(ID3D12Device* Device, const size_t Size, const D3D12_INDIRECT_ARGUMENT_TYPE Type) {
			Super::Create(Device, Size);
			const std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({.Type = Type }), };
			const D3D12_COMMAND_SIGNATURE_DESC CSD = { .ByteStride = static_cast<UINT>(Size), .NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs), .NodeMask = 0 };
			Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(CommandSignature));
			return *this;
		}
	public:
		COM_PTR<ID3D12CommandSignature> CommandSignature;
		IndirectBuffer& Create(ID3D12Device* Device, const D3D12_DRAW_INDEXED_ARGUMENTS& DIA) { return Create(Device, sizeof(DIA), D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED); }
		IndirectBuffer& Create(ID3D12Device* Device, const D3D12_DRAW_ARGUMENTS& DA) { return Create(Device, sizeof(DA), D3D12_INDIRECT_ARGUMENT_TYPE_DRAW); }
		IndirectBuffer& Create(ID3D12Device* Device, const D3D12_DISPATCH_ARGUMENTS& DA) { return Create(Device, sizeof(DA), D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH); }
#pragma region MESH_SHADER
		IndirectBuffer& Create(ID3D12Device* Device, const D3D12_DISPATCH_MESH_ARGUMENTS& DMA) { return Create(Device, sizeof(DMA), D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH); }
#pragma endregion
#pragma region RAYTRACING
		IndirectBuffer& Create(ID3D12Device* Device, const D3D12_DISPATCH_RAYS_DESC& DRD) { return Create(Device, sizeof(DRD), D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS); }
#pragma endregion
	};
	class ConstantBuffer : public UploadResource
	{
	private:
		using Super = UploadResource;
	public:
		ConstantBuffer& Create(ID3D12Device* Device, const size_t Size, const void* Source = nullptr) {
			Super::Create(Device, RoundUp256(Size), Source);
			return *this;
		}
	};
	//class ShaderResourceBuffer : public UploadResource
	//{
	//private:
	//	using Super = UploadResource;
	//public:
	//	D3D12_SHADER_RESOURCE_VIEW_DESC View;
	//	void Create(ID3D12Device* Device, const size_t Size, const void* Source, const size_t Stride) {
	//		Super::Create(Device, Size, Source);
	//		View = D3D12_SHADER_RESOURCE_VIEW_DESC({ 
	//			.Format = DXGI_FORMAT_UNKNOWN,
	//			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
	//			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	//			.Buffer = D3D12_BUFFER_SRV({.FirstElement = 0, .NumElements = static_cast<UINT>(Size / Stride), .StructureByteStride = static_cast<UINT>(Stride), .Flags = D3D12_BUFFER_SRV_FLAG_NONE })
	//		});
	//	}
	//};
	//class UnorderedAccessBuffer : public ResourceBase
	//{
	//public:
	//	D3D12_UNORDERED_ACCESS_VIEW_DESC View;
	//	void Create(ID3D12Device* Device, const size_t Size, const void* Source, const size_t Stride) {
	//		//D3D12_CPU_PAGE_PROPERTY_WRITE_BACK
	//		DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, RoundUp256(Size), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_CUSTOM, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, Source);
	//		View = D3D12_UNORDERED_ACCESS_VIEW_DESC({
	//			.Format = DXGI_FORMAT_UNKNOWN,
	//			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
	//			.Buffer = D3D12_BUFFER_UAV({.FirstElement = 0, .NumElements = static_cast<UINT>(Size / Stride), .StructureByteStride = static_cast<UINT>(Stride), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE })
	//		});
	//	}
	//};
	class TextureBase : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const DXGI_FORMAT Format) {
			DX::CreateTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, Format, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
		}
	};
	class Texture : public TextureBase
	{
	private:
		using Super = TextureBase;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const DXGI_FORMAT Format) {
			DX::CreateTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, Format, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
			//!< 基本 TEXTURE2D, TEXTURE2DARRAY として扱う、それ以外で使用する場合は明示的に上書きして使う
			SRV = DepthOrArraySize == 1 ?
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = Resource->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = Resource->GetDesc().MipLevels, .FirstArraySlice = 0, .ArraySize = DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) });
		}
	};
	class DepthTexture : public TextureBase
	{
	private:
		using Super = TextureBase;
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
	class UnorderedAccessTexture : public TextureBase
	{
	private:
		using Super = TextureBase;
	public:
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAV;
		void Create(ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const DXGI_FORMAT Format) {
			DX::CreateTextureResource(COM_PTR_PUT(Resource), Device, Width, Height, DepthOrArraySize, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			UAV = DepthOrArraySize == 1 ?
				D3D12_UNORDERED_ACCESS_VIEW_DESC({ .Format = Format, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D, .Texture2D = D3D12_TEX2D_UAV({.MipSlice = 0, .PlaneSlice = 0}) }) :
				D3D12_UNORDERED_ACCESS_VIEW_DESC({ .Format = Format, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY, .Texture2DArray = D3D12_TEX2D_ARRAY_UAV({.MipSlice = 0, .FirstArraySlice = 0, .ArraySize = DepthOrArraySize, .PlaneSlice = 0}) });
		}
	};

	class DefaultStructuredBuffer : public DefaultResource
	{
	private:
		using Super = DefaultResource;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		DefaultStructuredBuffer& Create(ID3D12Device* Dev, const size_t Size, const size_t Stride) {
			Super::Create(Dev, Size);
			SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = D3D12_BUFFER_SRV({
					.FirstElement = 0,
					.NumElements = static_cast<UINT>(Size / Stride),
					.StructureByteStride = static_cast<UINT>(Stride),
					.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
				})
				});
			return *this;
		}
	};

#pragma region MESH_SHADER
#pragma warning(push)
#pragma warning(disable : 4324)
	template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE T, typename U>
	class alignas(void*) PIPELINE_MESH_STATE_STREAM_SUBOBJECT 
	{
	public:
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT() : Type(T) {}
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT(const U& rhs) : Type(T), Value(rhs) {}
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
		U Value;
	};
#pragma warning(pop)
	struct PIPELINE_MESH_STATE_STREAM 
	{
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*> pRootSignature;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE> AS;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE> MS;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE> PS;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC> BlendState;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, UINT> SampleMask;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC> RasterizerState;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, D3D12_DEPTH_STENCIL_DESC1> DepthStencilState;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY> RTVFormats;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT> DSVFormat;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC> SampleDesc;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, UINT> NodeMask;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE> CachedPSO;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS> Flags;
		PIPELINE_MESH_STATE_STREAM_SUBOBJECT<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, D3D12_VIEW_INSTANCING_DESC> ViewInstancingDesc;
	};
#pragma endregion

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnPreDestroy() override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override { Super::OnDestroy(hWnd, hInstance); }

	[[nodiscard]] static const char* GetFormatChar(const DXGI_FORMAT Format);

	[[nodiscard]] static std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT3(data(lhs));
		const auto r = DirectX::XMFLOAT3(data(rhs));
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&l), DirectX::XMLoadFloat3(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2] };
	}
	[[nodiscard]] static std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT4(data(lhs));
		const auto r = DirectX::XMFLOAT4(data(rhs));
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&l), DirectX::XMLoadFloat4(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2], v.m128_f32[3] };
	}

#pragma region MESH_SHADER
	[[nodiscard]] static bool HasMeshShaderSupport(ID3D12Device* Device) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 FDO7;
		return SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, reinterpret_cast<void*>(&FDO7), sizeof(FDO7))) && D3D12_MESH_SHADER_TIER_NOT_SUPPORTED != FDO7.MeshShaderTier;
	}
#pragma endregion

protected:
	static void ResourceBarrier(ID3D12GraphicsCommandList* GCL, 
		ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After) {
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
	static void ResourceBarrier2(ID3D12GraphicsCommandList* GCL,
		ID3D12Resource* Resource0, const D3D12_RESOURCE_STATES Before0, const D3D12_RESOURCE_STATES After0,
		ID3D12Resource* Resource1, const D3D12_RESOURCE_STATES Before1, const D3D12_RESOURCE_STATES After1) {
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Resource0,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = Before0, .StateAfter = After0
				})
			}),
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Resource1,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = Before1, .StateAfter = After1
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
	static void CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const size_t Size, const D3D12_RESOURCE_FLAGS RF, const D3D12_HEAP_TYPE HT, const D3D12_RESOURCE_STATES RS, const void* Source = nullptr);
	static void CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const std::vector<D3D12_SUBRESOURCE_DATA>& SRDs, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizeInBytes, const UINT64 TotalBytes);
	static void CreateTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, DXGI_FORMAT Format, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS);
	static void CreateRenderTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, const D3D12_CLEAR_VALUE& CV, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS);
	static void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range = nullptr) {
		if (nullptr != Resource && Size && nullptr != Source) [[likely]] {
			BYTE* Data;
			VERIFY_SUCCEEDED(Resource->Map(0, Range, reinterpret_cast<void**>(&Data))); {
				memcpy(Data, Source, Size);
			} Resource->Unmap(0, nullptr);
		}
	}

public:
#ifdef USE_DXC
	static bool CompileShader(LPCWSTR ShaderPath, LPCWSTR Target, ID3DBlob** Blob);
	static bool CompileShader_VS(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("vs_6_6"), Blob); }
	static bool CompileShader_PS(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("ps_6_6"), Blob); }
	static bool CompileShader_DS(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("ds_6_6"), Blob); }
	static bool CompileShader_HS(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("hs_6_6"), Blob); }
	static bool CompileShader_GS(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("gs_6_6"), Blob); }
	static bool CompileShader_LIB(LPCWSTR ShaderPath, ID3DBlob** Blob) { return CompileShader(ShaderPath, TEXT("lib_6_8"), Blob); }
#endif

#pragma region COMMAND
	//static void PopulateCopyBufferRegionCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	static void PopulateCopyTextureRegionCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	static void ExecuteAndWait(ID3D12CommandQueue* CQ, ID3D12CommandList* CL, ID3D12Fence* Fence);
	static void PopulateBeginRenderTargetCommand(ID3D12GraphicsCommandList* GCL, ID3D12Resource* RenderTarget) {
		const std::array RBs = {
			//!< RenderTarget : COPY_SOURCE -> UNORDERED_ACCESS
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = RenderTarget,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE, .StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
	static void PopulateEndRenderTargetCommand(ID3D12GraphicsCommandList* GCL, ID3D12Resource* RenderTarget, ID3D12Resource* SwapChain) {
		{
			const std::array RBs = {
				//!< RenderTarget : UNORDERED_ACCESS -> COPY_SOURCE
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RenderTarget, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS, .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE })
				 }),
				//!< SwapChain : PRESENT -> COPY_DEST
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SwapChain, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST })
				})
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}

		GCL->CopyResource(SwapChain, RenderTarget);

		{
			const std::array RBs = {
				//!< SwapChain : COPY_DEST -> PRESENT
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
						.pResource = SwapChain,
						.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = D3D12_RESOURCE_STATE_PRESENT
					})
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
	}
#pragma endregion

#pragma region MARKER
	//!< #DX_TODO PIX 関連
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName([[maybe_unused]] ID3D12DeviceChild* Resource, [[maybe_unused]] const std::wstring_view Name) {
#ifdef USE_PIX
		Resource->SetName(data(Name));
#endif
	}
#pragma endregion

	virtual void CreateDevice(HWND hWnd);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);

	virtual void CreateCommandQueue();

	virtual void CreateFence() {
		VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(GraphicsFence)));
		VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(ComputeFence)));
		LOG_OK();
	}

	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void GetSwapChainResource();
	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);

	void CreateDirectCommandList(const UINT Count);
	virtual void CreateDirectCommandList() {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		CreateDirectCommandList(SCD.BufferCount);
	}
	void CreateBundleCommandList(const UINT Count);
	virtual void CreateBundleCommandList() {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		CreateBundleCommandList(SCD.BufferCount);
	}
	virtual void CreateComputeCommandList();
	virtual void CreateCommandList() {
		CreateDirectCommandList();
		CreateBundleCommandList();
		CreateComputeCommandList();
	}

	virtual void LoadScene() {}

	virtual void CreateGeometry() {}

	virtual void CreateConstantBuffer() {}

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);

	template<typename T = D3D12_ROOT_PARAMETER1> void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<T>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, const std::filesystem::path& Path);
	virtual void CreateRootSignature();

	virtual void CreateDescriptor() {}
	virtual void CreateShaderTable() {}

	virtual void CreateVideo();

	virtual void ProcessShaderReflection(ID3DBlob* Blob);
	virtual void SetBlobPart(COM_PTR<ID3DBlob>& Blob);
	virtual void GetBlobPart(ID3DBlob* Blob);
	virtual void StripShader(COM_PTR<ID3DBlob>& Blob);
	class PipelineLibrarySerializer
	{
	public:
		PipelineLibrarySerializer(ID3D12Device* Dev, const std::filesystem::path& Path) : Device(Dev), FilePath(Path) {
#ifdef ALWAYS_REBUILD_PIPELINE
			DeleteFile(data(FilePath.wstring()));
#endif
			COM_PTR<ID3D12Device1> Device1;
			VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device1)));

			COM_PTR<ID3DBlob> Blob;
			if (SUCCEEDED(D3DReadFileToBlob(data(FilePath.wstring()), COM_PTR_PUT(Blob))) && Blob->GetBufferSize()) {
				Logf("PipelineLibrarySerializer : Reading PipelineLibrary = %ls\n", data(FilePath.string()));
				//!< ファイルが読めた場合は PipelineLinrary へ読み込む (If file is read, load to PipplineLibrary)
				VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
				IsLoaded = true;
			}
			else {
				Log("PipelineLibrarySerializer : Creating PipelineLibrary\n");
				//!< ファイルが読めなかった場合は書き込み用に新規作成 (If file is not read, create new for write)
				VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
			}
		}
		virtual ~PipelineLibrarySerializer() {
			if (!IsLoaded) {
				Logf("PipelineLibrarySerializer : Writing PipelineLibrary = %ls\n", data(FilePath.string()));
				//!< ファイルへ書き込む (Write to file)
				const auto Size = PipelineLibrary->GetSerializedSize();
				if (Size) {
					COM_PTR<ID3DBlob> Blob;
					VERIFY_SUCCEEDED(D3DCreateBlob(Size, COM_PTR_PUT(Blob)));
					VERIFY_SUCCEEDED(PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size));
					VERIFY_SUCCEEDED(D3DWriteBlobToFile(COM_PTR_GET(Blob), data(FilePath.wstring()), TRUE));
				}
			}
			LOG_OK();
		}
		ID3D12PipelineLibrary* GetPipelineLibrary() const { return COM_PTR_GET(PipelineLibrary); }
		bool IsLoadSucceeded() const { return IsLoaded; }
	private:
		ID3D12Device* Device;
		std::filesystem::path FilePath;
		COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
		bool IsLoaded = false;
	};
	virtual void CreatePipelineState() {}
	//!< VS, PS, DS, HS, GS
	static void CreatePipelineStateVsPsDsHsGs(COM_PTR<ID3D12PipelineState>& PST,
		ID3D12Device* Device, ID3D12RootSignature* RS,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT,
		const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
		const D3D12_RASTERIZER_DESC& RD,
		const D3D12_DEPTH_STENCIL_DESC& DSD,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, 
		const std::vector<DXGI_FORMAT>& RTVFormats,
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);
	//!< AS, MS, PS
	static void CreatePipelineStateAsMsPs(COM_PTR<ID3D12PipelineState>& PST,
		ID3D12Device* Device, ID3D12RootSignature* RS,
		const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
		const D3D12_RASTERIZER_DESC& RD,
		const D3D12_DEPTH_STENCIL_DESC1& DSD,
		const D3D12_SHADER_BYTECODE AS, const D3D12_SHADER_BYTECODE MS, const D3D12_SHADER_BYTECODE PS,
		const std::vector<DXGI_FORMAT>& RTVFormats,
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);

	virtual void CreateTexture() {}
	virtual void CreateTextureArray1x1(const std::vector<UINT32>& Colors, const D3D12_RESOURCE_STATES RS);
	virtual void CreateStaticSampler() {}

	virtual void PopulateBundleCommandList([[maybe_unused]] const size_t i) {}
	virtual void PopulateCommandList([[maybe_unused]] const size_t i) {}

	virtual UINT GetCurrentBackBufferIndex() const { return SwapChain->GetCurrentBackBufferIndex(); }
	virtual void DrawFrame([[maybe_unused]] const UINT i) {}
	static void WaitForFence(ID3D12CommandQueue* CQ, ID3D12Fence* Fence);
	virtual void SubmitGraphics(const UINT i);
	virtual void SubmitCompute(const UINT i);
	virtual void Present();
	virtual void Draw();
	virtual void Dispatch();
	
protected:
	std::vector<std::thread> Threads;

#ifdef USE_PIX
	COM_PTR<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif

	COM_PTR<IDXGIFactory4> Factory;
	COM_PTR<IDXGIAdapter> Adapter;
	COM_PTR<IDXGIOutput> Output;
	COM_PTR<ID3D12Device> Device;

	COM_PTR<ID3D12CommandQueue> GraphicsCommandQueue;
	COM_PTR<ID3D12CommandQueue> ComputeCommandQueue;

	/**
	フェンス		... デバイスとホスト(GPUとCPU)の同期、コマンドバッファ間の同期
	セマフォ		... 存在しない、フェンスで代用 ... VKには存在
	バリア		... コマンドバッファ内の同期
	イベント		... 存在しない ... VKには存在
	*/
	COM_PTR<ID3D12Fence> GraphicsFence;
	COM_PTR<ID3D12Fence> ComputeFence;

	COM_PTR<IDXGISwapChain4> SwapChain;
	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap; //!< D3D12_DESCRIPTOR_HEAP_TYPE_RTV : スワップチェインRTVは別扱いにしている (Manage swapchain RTV separately)
	struct SwapChainBackBuffer 
	{
		COM_PTR<ID3D12Resource> Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE Handle;
	}; 
	std::vector<SwapChainBackBuffer> SwapChainBackBuffers;

	std::vector<COM_PTR<ID3D12CommandAllocator>> DirectCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> DirectCommandLists;
	std::vector<COM_PTR<ID3D12CommandAllocator>> BundleCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> BundleCommandLists;
	std::vector<COM_PTR<ID3D12CommandAllocator>> ComputeCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> ComputeCommandLists;

	std::vector<DXGI_SAMPLE_DESC> SampleDescs;

	std::vector<VertexBuffer> VertexBuffers;
	std::vector<IndexBuffer> IndexBuffers;
	std::vector<IndirectBuffer> IndirectBuffers;
	std::vector<ConstantBuffer> ConstantBuffers;

	std::vector<Texture> Textures;
	std::vector<DepthTexture> DepthTextures;
	std::vector<RenderTexture> RenderTextures;
	std::vector<UnorderedAccessTexture> UnorderedAccessTextures;

	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

	std::vector<COM_PTR<ID3D12RootSignature>> RootSignatures;
	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	std::vector<COM_PTR<ID3D12PipelineState>> PipelineStates;

	std::vector<std::pair<COM_PTR<ID3D12DescriptorHeap>, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>>> CbvSrvUavDescs;
	std::vector<std::pair<COM_PTR<ID3D12DescriptorHeap>, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>> RtvDescs;
	std::vector<std::pair<COM_PTR<ID3D12DescriptorHeap>, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>> DsvDescs;

	std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> UnorderedAccessViewDescs;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	COM_PTR<ID3D12VideoDecoder> VideoDecoder;
	COM_PTR<ID3D12VideoDecoderHeap> VideoDecoderHeap;
	
protected:
	const D3D12_SHADER_BYTECODE NullSBC = { .pShaderBytecode = nullptr, .BytecodeLength = 0 };
};

#ifdef DEBUG_STDOUT
static std::ostream& operator<<(std::ostream& lhs, const DirectX::XMVECTOR& rhs) { for (auto i = 0; i < 4; ++i) { lhs << rhs.m128_f32[i] << ", "; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const DirectX::XMMATRIX& rhs) { for (auto i = 0; i < 4; ++i) { lhs << rhs.r[i]; } lhs << std::endl; return lhs; }

static std::ostream& operator<<(std::ostream& lhs, IDXGIOutput* rhs) {
#ifdef USE_HDR
	COM_PTR<IDXGIOutput6> DO6;
	COM_PTR_AS(rhs, DO6);
	DXGI_OUTPUT_DESC1 OD;
	VERIFY_SUCCEEDED(DO6->GetDesc1(&OD));
	//!< Windows側で "Play HDR game and apps" を有効にする設定が必要 (Need to enable "Play HDR game and apps" in windows settings)
	assert(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 == OD.ColorSpace && "HDR not supported");
#else
	DXGI_OUTPUT_DESC OD;
	VERIFY_SUCCEEDED(rhs->GetDesc(&OD));
#endif
	Win::Logf(TEXT("\t\t\t%s\n"), OD.DeviceName);
	Win::Logf(TEXT("\t\t\t%d x %d\n"), OD.DesktopCoordinates.right - OD.DesktopCoordinates.left, OD.DesktopCoordinates.bottom - OD.DesktopCoordinates.top);
	switch (OD.Rotation)
	{
		default: break;
		case DXGI_MODE_ROTATION_UNSPECIFIED: Win::Log("\t\t\tROTATION_UNSPECIFIED\n"); break;
		case DXGI_MODE_ROTATION_IDENTITY: Win::Log("\t\t\tROTATION_IDENTITY\n"); break;
		case DXGI_MODE_ROTATION_ROTATE90: Win::Log("\t\t\tROTATE90\n"); break;
		case DXGI_MODE_ROTATION_ROTATE180: Win::Log("\t\t\tROTATE180\n"); break;
		case DXGI_MODE_ROTATION_ROTATE270: Win::Log("\t\t\tROTATE270\n"); break;
	}
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, IDXGIAdapter* rhs) {
	DXGI_ADAPTER_DESC AD;
	VERIFY_SUCCEEDED(rhs->GetDesc(&AD));
	Win::Logf(TEXT("\t%s\n"), AD.Description);
	Win::Logf(TEXT("\t\tDedicatedVideoMemory = %lld\n"), AD.DedicatedVideoMemory);
	Win::Logf(TEXT("\t\tDedicatedSystemMemory = %lld\n"), AD.DedicatedSystemMemory);
	Win::Logf(TEXT("\t\tSharedSystemMemory = %lld\n"), AD.SharedSystemMemory);

	Win::Log("\t\t[ Outputs ]\n");
	COM_PTR<IDXGIOutput> DO;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != rhs->EnumOutputs(i, COM_PTR_PUT(DO)); ++i) {
		std::cout << COM_PTR_GET(DO);
		COM_PTR_RESET(DO);
	}
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, IDXGIFactory4* rhs) {
	Win::Log("[ Aadapters ]\n");
	COM_PTR<IDXGIAdapter> DA;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != rhs->EnumAdapters(i, COM_PTR_PUT(DA)); ++i) {
		std::cout << COM_PTR_GET(DA);
		COM_PTR_RESET(DA);
	}
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, const DXGI_MODE_DESC& rhs) {
	Win::Logf("\t\t\t\t%d x %d @ %d, ", rhs.Width, rhs.Height, rhs.RefreshRate.Numerator / rhs.RefreshRate.Denominator);
#define SCANLINE_ORDERING_ENTRY(slo) case DXGI_MODE_SCANLINE_ORDER_##slo: Win::Logf("SCANLINE_ORDER_%s, ", #slo); break;
	switch (rhs.ScanlineOrdering) {
		default: assert(0 && "Unknown ScanlineOrdering"); break;
		SCANLINE_ORDERING_ENTRY(UNSPECIFIED)
		SCANLINE_ORDERING_ENTRY(PROGRESSIVE)
		SCANLINE_ORDERING_ENTRY(UPPER_FIELD_FIRST)
		SCANLINE_ORDERING_ENTRY(LOWER_FIELD_FIRST)
	}
#undef SCANLINE_ORDERING_ENTRY

#define SCALING_ENTRY(s) case DXGI_MODE_SCALING_##s: Win::Logf("SCALING_%s", #s); break;
	switch (rhs.Scaling) {
		default: assert(0 && "Unknown Scaling"); break;
		SCALING_ENTRY(UNSPECIFIED)
		SCALING_ENTRY(CENTERED)
		SCALING_ENTRY(STRETCHED)
	}
#undef SCALING_ENTRY
	Win::Log("\n");
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, ID3D12ShaderReflection* rhs) {
	D3D12_SHADER_DESC SD;
	VERIFY_SUCCEEDED(rhs->GetDesc(&SD));
	if (0 < SD.ConstantBuffers) { Win::Log("\tConstantBuffers\n"); }
	for (UINT i = 0; i < SD.ConstantBuffers; ++i) {
		const auto CB = rhs->GetConstantBufferByIndex(i);
		D3D12_SHADER_BUFFER_DESC SBD;
		VERIFY_SUCCEEDED(CB->GetDesc(&SBD));
		Win::Logf("\t\t%s\n", SBD.Name);
		for (UINT j = 0; j < SBD.Variables; ++j) {
			const auto SRV = CB->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC SVD;
			VERIFY_SUCCEEDED(SRV->GetDesc(&SVD));
			D3D12_SHADER_TYPE_DESC STD;
			VERIFY_SUCCEEDED(SRV->GetType()->GetDesc(&STD));
			Win::Logf("\t\t\t%s\t%s\n", SVD.Name, STD.Name);
		}
	}

	if (0 < SD.BoundResources) { Win::Log("\tBoundResources\n"); }
	for (UINT i = 0; i < SD.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC SIBD;
		VERIFY_SUCCEEDED(rhs->GetResourceBindingDesc(i, &SIBD));
		Win::Logf("\t\t%s\n", SIBD.Name);
		Win::Log("\t\t\tInput = ");
#define SHADER_INPUT_TYPE_ENTRY(X) case D3D_SIT_##X: Win::Log(#X); break;
		switch (SIBD.Type) {
		SHADER_INPUT_TYPE_ENTRY(CBUFFER)
		SHADER_INPUT_TYPE_ENTRY(TBUFFER)
		SHADER_INPUT_TYPE_ENTRY(TEXTURE)
		SHADER_INPUT_TYPE_ENTRY(SAMPLER)
		SHADER_INPUT_TYPE_ENTRY(UAV_RWTYPED)
		SHADER_INPUT_TYPE_ENTRY(STRUCTURED)
		SHADER_INPUT_TYPE_ENTRY(UAV_RWSTRUCTURED)
		SHADER_INPUT_TYPE_ENTRY(BYTEADDRESS)
		SHADER_INPUT_TYPE_ENTRY(UAV_RWBYTEADDRESS)
		SHADER_INPUT_TYPE_ENTRY(UAV_APPEND_STRUCTURED)
		SHADER_INPUT_TYPE_ENTRY(UAV_CONSUME_STRUCTURED)
		SHADER_INPUT_TYPE_ENTRY(UAV_RWSTRUCTURED_WITH_COUNTER)
		default: break;
		}
#undef SHADER_INPUT_TYPE_ENTRY
		0 < SIBD.BindCount ? Win::Logf("(%d-%d)", SIBD.BindPoint, SIBD.BindCount + SIBD.BindCount) : Win::Logf("(%d)", SIBD.BindPoint);
		Win::Log("\n");
		//!< ReturnType, Dimension はテクスチャの場合のみ
		if (D3D_SIT_TEXTURE == SIBD.Type) {
			Win::Log("\t\t\tReturn = ");
#define RETURN_TYPE_ENTRY(X) case D3D_RETURN_TYPE_##X: Win::Log(#X); break;
			switch (SIBD.ReturnType) {
			RETURN_TYPE_ENTRY(UNORM)
			RETURN_TYPE_ENTRY(SNORM)
			RETURN_TYPE_ENTRY(SINT)
			RETURN_TYPE_ENTRY(UINT)
			RETURN_TYPE_ENTRY(FLOAT)
			RETURN_TYPE_ENTRY(MIXED)
			RETURN_TYPE_ENTRY(DOUBLE)
			RETURN_TYPE_ENTRY(CONTINUED)
			default: break;
			}
#undef RETURN_TYPE_ENTRY
			Win::Log("\n");

			Win::Log("\t\t\tDimension = ");
#define DIMENSION_ENTRY(X) case D3D_SRV_DIMENSION_##X: Win::Log(#X); break;
			switch (SIBD.Dimension) {
			DIMENSION_ENTRY(UNKNOWN)
			DIMENSION_ENTRY(BUFFER)
			DIMENSION_ENTRY(TEXTURE1D)
			DIMENSION_ENTRY(TEXTURE1DARRAY)
			DIMENSION_ENTRY(TEXTURE2D)
			DIMENSION_ENTRY(TEXTURE2DARRAY)
			DIMENSION_ENTRY(TEXTURE2DMS)
			DIMENSION_ENTRY(TEXTURE2DMSARRAY)
			DIMENSION_ENTRY(TEXTURE3D)
			DIMENSION_ENTRY(TEXTURECUBE)
			DIMENSION_ENTRY(TEXTURECUBEARRAY)
			DIMENSION_ENTRY(BUFFEREX)
			default: break;
			}
#undef DIMENSION_ENTRY
			Win::Log("\n");
		}
	}

#define COMPONENT_TYPE_ENTRY(X) case D3D_REGISTER_COMPONENT_##X: Win::Log(#X); break;
#define LOG_SIGNATURE_PARAMETER_DESC(X)	Win::Logf("\t\t%s%d\t", X.SemanticName, X.SemanticIndex);\
	switch (X.ComponentType) {\
		COMPONENT_TYPE_ENTRY(UNKNOWN)\
		COMPONENT_TYPE_ENTRY(UINT32)\
		COMPONENT_TYPE_ENTRY(SINT32)\
		COMPONENT_TYPE_ENTRY(FLOAT32)\
		default: break;\
	}\
	Win::Logf("(%s%s%s%s)\n", (X.Mask & 1) ? "X" : "-", (X.Mask & 2) ? "Y" : "-", (X.Mask & 4) ? "Z" : "-", (X.Mask & 8) ? "W" : "-");

	if (0 < SD.InputParameters) { Win::Logf("\tInputParameters\n"); }
	for (UINT i = 0; i < SD.InputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(rhs->GetInputParameterDesc(i, &SPD));
		LOG_SIGNATURE_PARAMETER_DESC(SPD);
	}
	if (0 < SD.OutputParameters) { Win::Logf("\tOutputParameters\n"); }
	for (UINT i = 0; i < SD.OutputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(rhs->GetOutputParameterDesc(i, &SPD));
		LOG_SIGNATURE_PARAMETER_DESC(SPD);
	}
#undef LOG_SIGNATURE_PARAMETER_DESC
#undef COMPONENT_TYPE_ENTRY

	return lhs;
}
#endif