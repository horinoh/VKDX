#pragma once
 
//!< USE_WINRT ����`����Ȃ��ꍇ�� WRL ���g�p�����AWINRT �ł� �vC++17�ȍ~ (If USE_WINRT is not defined, WRL will be used, WINRT require C++17 or later)
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

#include <initguid.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_6.h>
#include <DirectXMath.h>

#ifndef VERIFY_SUCCEEDED
#ifdef _DEBUG
//#define VERIFY_SUCCEEDED(X) { const auto HR = (X); if(FAILED(HR)) { OutputDebugStringA(data(std::system_category().message(HR) + "\n")); DEBUG_BREAK(); } }
#define VERIFY_SUCCEEDED(X) { const auto HR = (X); if(FAILED(HR)) { MessageBoxA(nullptr, data(std::system_category().message(HR)), "", MB_OK); DEBUG_BREAK(); /*throw std::runtime_error("");*/ } }
#else
#define VERIFY_SUCCEEDED(X) (X) 
#endif
#endif

//!< �\�t�g�E�G�A���X�^���C�U (Software rasterizer)
//#define USE_WARP
#define USE_STATIC_SAMPLER //!< [ TextureDX ] VK:USE_IMMUTABLE_SAMPLER����
//!< HLSL���烋�[�g�V�O�l�`�����쐬���� (Create root signature from HLSL)
//#define USE_HLSL_ROOTSIGNATRUE //!< [ TriangleDX ]
//#define USE_BUNDLE //!< [ ParametricSurfaceDX ] VK:USE_SECONDARY_COMMAND_BUFFER����
//#define USE_ROOT_CONSTANTS //!< [ TriangleDX ] VK:USE_PUSH_CONSTANTS����
//#define USE_GAMMA_CORRECTION
#define USE_SHADER_BLOB_PART //!< [ TriangleDX ]
#define USE_DXC

#ifdef USE_DXC
#include <dxcapi.h>
//!< DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h �̂��̂������ֈڐA (Same as defined in DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h)
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8  | (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24)
#endif

#define SHADER_ROOT_ACCESS_DENY_ALL (D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_VS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_GS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
#define SHADER_ROOT_ACCESS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))
#define SHADER_ROOT_ACCESS_DS_GS_PS (SHADER_ROOT_ACCESS_DENY_ALL & ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS))

/**
@brief 32 bit �J���[ DirectX::PackedVector::XMCOLOR
@note ARGB���C�A�E�g

XMCOLOR Color32;
XMVECTOR Color128;

@note 128 bit �J���[ �� 32 bit �J���[
DirectX::PackedVector::XMStoreColor(&Color32, Color128);

@note 32 bit �J���[ �� 128 bit �J���[
Color128 = DirectX::PackedVector::XMLoadColor(Color32);
*/
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

#include <comdef.h>
#include <system_error>

//!< _DEBUG �ł���Ή������Ȃ��Ă� PIX �g�p�\�ARelease �� PIX ���g�p�������悤�ȏꍇ�� USE_PIX ���`���� (In case want to use pix in Release build, define USE_PIX)
//!< �\�����[�V�����E�N���b�N - �\�����[�V������NuGet�p�b�P�[�W�̊Ǘ� - �Q�ƃ^�u - WinPixEventRuntime�Ō��� - �v���W�F�N�g��I������PIX���C���X�g�[�����Ă�������
//#define USE_PIX
#include <pix3.h>
//!< �v���O��������L���v�`�����s�������ꍇ (Capture in program code)
#if defined(_DEBUG) || defined(USE_PIX)
#include <DXProgrammableCapture.h>
#endif

#include "Cmn.h"
#include "Win.h"

/**
���\�[�X���쐬���ꂽ�� MakeResident() ����A�j�����ꂽ�� Evict() �����B
�A�v�����疾���I�ɂ�����s�������ꍇ�͈ȉ��̂悤�ɂ���
ID3D12Resource* Resource;
const std::vector<ID3D12Pageable*> Pageables = { Resource };
Device->MakeResident(static_cast<UINT>(size(Pageables)), data(Pageables));
Device->Evict(static_cast<UINT>(size(Pageables)), data(Pageables));
*/

/**
CommandList�ACommandAllocator �̓X���b�h�Z�[�t�ł͂Ȃ��̂Ŋe�X���b�h���Ɏ��K�v������
CommandQueue �̓X���b�h�t���[�Ŋe�X���b�h����g�p�\
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
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const size_t Size, ID3D12Resource* Upload) {
			{
				const std::array RBs = {
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
							.pResource = COM_PTR_GET(Resource),
							.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
							.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST
						})
					})
				};
				GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}
			GCL->CopyBufferRegion(COM_PTR_GET(Resource), 0, Upload, 0, Size);
			{
				const std::array RBs = {
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
							.pResource = COM_PTR_GET(Resource),
							.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
							.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ
						})
					})
				};
				GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}
		}
		void ExecuteCopyCommand(ID3D12Device* Device, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const size_t Size, const void* Source) {
			UploadResource Upload;
			Upload.Create(Device, Size, Source);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Size, COM_PTR_GET(Upload.Resource));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fence);
		}
	};
	class DefaultResource : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const size_t Size) { DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ); }
	};
	class UploadResource : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const size_t Size, const void* Source = nullptr) { DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source); }
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
		void Create(ID3D12Device* Device, const size_t Size, const void* Source = nullptr) {
			Super::Create(Device, RoundUp256(Size), Source);
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
			//!< ��{ TEXTURE2D, TEXTURE2DARRAY �Ƃ��Ĉ����A����ȊO�Ŏg�p����ꍇ�͖����I�ɏ㏑�����Ďg��
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
#pragma region RAYTRACING
	class ASCompaction
	{
	public:
		COM_PTR<ID3D12Resource> Info;
		COM_PTR<ID3D12Resource> Read;
		virtual ASCompaction& Create(ID3D12Device* Device) {
			constexpr auto Size = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC);
			DX::CreateBufferResource(COM_PTR_PUT(Info), Device, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			DX::CreateBufferResource(COM_PTR_PUT(Read), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST);
			return *this;
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL) {
			//!< ���[�h�o�b�N�ւ̃R�s�[�R�}���h�𔭍s���� 
			const std::array RBs = {
				D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = COM_PTR_GET(Info),
					.Subresource = 0,
					.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS, .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE
					})
				})
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			GCL->CopyResource(COM_PTR_GET(Read), COM_PTR_GET(Info));
		}
		UINT64 GetSize() {
			//!< ���ʂ����[�h�o�b�N�փR�s�[������Ɏg�p���邱��
			UINT64 Size;
			BYTE* Data;
			Read->Map(0, nullptr, reinterpret_cast<void**>(&Data)); {
				Size = reinterpret_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC*>(Data)->CompactedSizeInBytes;
			} Read->Unmap(0, nullptr);
			return Size;
		}
	};
	class AccelerationStructureBuffer : public ResourceBase
	{
	public:
		virtual AccelerationStructureBuffer& Create(ID3D12Device* Device, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
			return *this;
		}
		void PopulateBuildCommand(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12Resource* Scratch, ASCompaction* Compaction = nullptr) {
			COM_PTR<ID3D12GraphicsCommandList4> GCL4;
			VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
			const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD = {
				.DestAccelerationStructureData = Resource->GetGPUVirtualAddress(),
				.Inputs = BRASI,
				.SourceAccelerationStructureData = 0,
				.ScratchAccelerationStructureData = Scratch->GetGPUVirtualAddress()
			};
			if (nullptr != Compaction) {
				//!< �R���p�N�V�����T�C�Y���擾����ݒ�
				const std::array RASPIDs = {
					D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC({.DestBuffer = Compaction->Info->GetGPUVirtualAddress(), .InfoType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE}),
				};
				GCL4->BuildRaytracingAccelerationStructure(&BRASD, static_cast<UINT>(size(RASPIDs)), data(RASPIDs));
				
				//!< ���ʂ����[�h�o�b�N�փR�s�[
				Compaction->PopulateCopyCommand(GCL);
			}
			else {
				constexpr std::array<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC, 0> RASPIDs = {};
				GCL4->BuildRaytracingAccelerationStructure(&BRASD, static_cast<UINT>(size(RASPIDs)), data(RASPIDs));
			}
		}
		void ExecuteBuildCommand(ID3D12Device* Device, const size_t Size, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, ASCompaction* Compaction = nullptr) {
			ScratchBuffer SB;
			SB.Create(Device, Size);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateBuildCommand(BRASI, GCL, COM_PTR_GET(SB.Resource), Compaction);
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, static_cast<ID3D12CommandList*>(GCL), Fence);
		}
	};
	class BLAS : public AccelerationStructureBuffer 
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		virtual BLAS& Create(ID3D12Device* Device, const size_t Size) {
			Super::Create(Device, Size);
			return *this;
		}
		void PopulateBarrierCommand(ID3D12GraphicsCommandList* GCL) {
			const std::array RBs = {
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.UAV = D3D12_RESOURCE_UAV_BARRIER({.pResource = COM_PTR_GET(Resource)})
				})
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
		void ExecuteCopyCommand(ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, ID3D12Resource* Src) {
			COM_PTR<ID3D12GraphicsCommandList4> GCL4;
			VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				GCL4->CopyRaytracingAccelerationStructure(Resource->GetGPUVirtualAddress(), Src->GetGPUVirtualAddress(), D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT);
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, static_cast<ID3D12CommandList*>(GCL), Fence);
		}
	};
	class TLAS : public AccelerationStructureBuffer 
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		virtual TLAS& Create(ID3D12Device* Device, const size_t Size) override {
			Super::Create(Device, Size);
			SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.RaytracingAccelerationStructure = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV({.Location = Resource->GetGPUVirtualAddress()})
			});
			return *this;
		}
	};
	class StructuredBuffer : public UploadResource
	{
	private:
		using Super = UploadResource;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		void Create(ID3D12Device* Device, const size_t Size, const size_t Stride, const void* Source) { 
			Super::Create(Device, Size, Source);
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
		}
	};
	class ScratchBuffer : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Device, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}; 
	class ShaderTable : public ResourceBase 
	{
	private:
		using Super = ResourceBase;
	public:
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE Range;
		ShaderTable& Create(ID3D12Device* Device, const size_t Size, const size_t Stride) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
			Range = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({
				.StartAddress = Resource->GetGPUVirtualAddress(),
				.SizeInBytes = Size,
				.StrideInBytes = Stride //!< ����1�̏ꍇ�� 0 �ł��ǂ�
			});
			return *this;
		}
		BYTE* Map() {
			BYTE* Data;
			Resource->Map(0, nullptr, reinterpret_cast<void**>(&Data));
			return Data;
		}
		void Unmap() { Resource->Unmap(0, nullptr); }
	};
#pragma endregion

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
	virtual void OnPreDestroy(HWND hWnd, HINSTANCE hInstance) override;
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

#pragma region RAYTRACING
	[[nodiscard]] static bool HasRaytracingSupport(ID3D12Device* Device) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 FDO5;
		return SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, reinterpret_cast<void*>(&FDO5), sizeof(FDO5))) && D3D12_RAYTRACING_TIER_NOT_SUPPORTED != FDO5.RaytracingTier;
	}
#pragma endregion
#pragma region MESH_SHADER
	[[nodiscard]] static bool HasMeshShaderSupport(ID3D12Device* Device) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 FDO7;
		return SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, reinterpret_cast<void*>(&FDO7), sizeof(FDO7))) && D3D12_MESH_SHADER_TIER_NOT_SUPPORTED != FDO7.MeshShaderTier;
	}
#pragma endregion

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

public:
#pragma region COMMAND
	//static void PopulateCopyBufferRegionCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	static void PopulateCopyTextureRegionCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	static void ExecuteAndWait(ID3D12CommandQueue* CQ, ID3D12CommandList* CL, ID3D12Fence* Fence);
#pragma endregion

#pragma region MARKER
	//!< #DX_TODO PIX �֘A
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName([[maybe_unused]] ID3D12DeviceChild* Resource, [[maybe_unused]] const std::wstring_view Name) {
#if defined(_DEBUG) || defined(USE_PIX)
		Resource->SetName(data(Name));
#endif
	}
#pragma endregion

	virtual void CreateDevice(HWND hWnd);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);

	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void GetSwapChainResource();
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);

	virtual void CreateCommandList();

	virtual void LoadScene() {}

	virtual void CreateGeometry() {}

	virtual void CreateConstantBuffer() {}

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);

	template<typename T = D3D12_ROOT_PARAMETER> void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<T>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path);
	virtual void CreateRootSignature();

	virtual void CreateDescriptor() {}
	virtual void CreateShaderTable() {}

	virtual void ProcessShaderReflection(ID3DBlob* Blob);
	virtual void SetBlobPart(COM_PTR<ID3DBlob>& Blob);
	virtual void GetBlobPart(ID3DBlob* Blob);
	virtual void StripShader(COM_PTR<ID3DBlob>& Blob);
#ifdef USE_PIPELINE_SERIALIZE
	class PipelineLibrarySerializer
	{
	public:
		PipelineLibrarySerializer(ID3D12Device* Dev, std::wstring_view Path) : Device(Dev), FilePath(Path) {
#ifdef ALWAYS_REBUILD_PIPELINE
			DeleteFile(data(FilePath));
#endif
			COM_PTR<ID3D12Device1> Device1;
			VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device1)));

			COM_PTR<ID3DBlob> Blob;
			if (SUCCEEDED(D3DReadFileToBlob(data(FilePath), COM_PTR_PUT(Blob))) && Blob->GetBufferSize()) {
				Logf("PipelineLibrarySerializer : Reading PipelineLibrary = %ls\n", data(FilePath));
				//!< �t�@�C�����ǂ߂��ꍇ�� PipelineLinrary �֓ǂݍ��� (If file is read, load to PipplineLibrary)
				VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
				IsLoaded = true;
			}
			else {
				Log("PipelineLibrarySerializer : Creating PipelineLibrary\n");
				//!< �t�@�C�����ǂ߂Ȃ������ꍇ�͏������ݗp�ɐV�K�쐬 (If file is not read, create new for write)
				VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
			}
		}
		virtual ~PipelineLibrarySerializer() {
			if (!IsLoaded) {
				Logf("PipelineLibrarySerializer : Writing PipelineLibrary = %ls\n", data(FilePath));
				//!< �t�@�C���֏������� (Write to file)
				const auto Size = PipelineLibrary->GetSerializedSize();
				if (Size) {
					COM_PTR<ID3DBlob> Blob;
					VERIFY_SUCCEEDED(D3DCreateBlob(Size, COM_PTR_PUT(Blob)));
					VERIFY_SUCCEEDED(PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size));
					VERIFY_SUCCEEDED(D3DWriteBlobToFile(COM_PTR_GET(Blob), data(FilePath), TRUE));
				}
			}
			LOG_OK();
		}
		ID3D12PipelineLibrary* GetPipelineLibrary() const { return COM_PTR_GET(PipelineLibrary); }
		bool IsLoadSucceeded() const { return IsLoaded; }
	private:
		ID3D12Device* Device;
		std::wstring FilePath;
		COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
		bool IsLoaded = false;
	};
#endif
	virtual void CreatePipelineState() {}
	static void CreatePipelineState_(COM_PTR<ID3D12PipelineState>& PST,
		ID3D12Device* Device, ID3D12RootSignature* RS,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT,
		const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
		const D3D12_RASTERIZER_DESC& RD,
		const D3D12_DEPTH_STENCIL_DESC& DSD,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, 
		const std::vector<DXGI_FORMAT>& RTVFormats,
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);
	static void CreatePipelineState__(COM_PTR<ID3D12PipelineState>& PST,
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

	virtual void PopulateCommandList([[maybe_unused]] const size_t i) {}

	virtual UINT GetCurrentBackBufferIndex() const { return SwapChain->GetCurrentBackBufferIndex(); }
	virtual void DrawFrame([[maybe_unused]] const UINT i) {}
	virtual void Draw();
	virtual void Dispatch();
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

	COM_PTR<IDXGISwapChain4> SwapChain;
	std::vector<COM_PTR<ID3D12Resource>> SwapChainResources;
	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap;					//!< D3D12_DESCRIPTOR_HEAP_TYPE_RTV : �X���b�v�`�F�C��RTV�͕ʈ����ɂ��Ă��� (Manage swapchain RTV separately)
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> SwapChainCPUHandles;

	std::vector<COM_PTR<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<COM_PTR<ID3D12CommandAllocator>> BundleCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> BundleGraphicsCommandLists;

	std::vector<VertexBuffer> VertexBuffers;
	std::vector<IndexBuffer> IndexBuffers;
	std::vector<IndirectBuffer> IndirectBuffers;
	std::vector<ConstantBuffer> ConstantBuffers;
#pragma region RAYTRACING
	std::vector<BLAS> BLASs;
	std::vector<TLAS> TLASs;
	std::vector<ShaderTable> ShaderTables;
#pragma endregion

	std::vector<Texture> Textures;
	std::vector<DepthTexture> DepthTextures;
	std::vector<RenderTexture> RenderTextures;
	std::vector<UnorderedAccessTexture> UnorderedAccessTextures;

	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

	std::vector<COM_PTR<ID3D12RootSignature>> RootSignatures;
	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	std::vector<COM_PTR<ID3D12PipelineState>> PipelineStates;
#pragma region RAYTRACING
	std::vector<COM_PTR<ID3D12StateObject>> StateObjects;
#pragma endregion

	std::vector<COM_PTR<ID3D12DescriptorHeap>> CbvSrvUavDescriptorHeaps;	//!< D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	std::vector<COM_PTR<ID3D12DescriptorHeap>> SamplerDescriptorHeaps;		//!< D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
	std::vector<COM_PTR<ID3D12DescriptorHeap>> RtvDescriptorHeaps;			//!< D3D12_DESCRIPTOR_HEAP_TYPE_RTV
	std::vector<COM_PTR<ID3D12DescriptorHeap>> DsvDescriptorHeaps;			//!< D3D12_DESCRIPTOR_HEAP_TYPE_DSV

	std::vector<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> CbvSrvUavGPUHandles;
	std::vector<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> SamplerGPUHandles;
	std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> RtvCPUHandles;
	std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> DsvCPUHandles;

	std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> UnorderedAccessViewDescs;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

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
	//!< Windows���� "Play HDR game and apps" ��L���ɂ���ݒ肪�K�v (Need to enable "Play HDR game and apps" in windows settings)
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
		//!< ReturnType, Dimension �̓e�N�X�`���̏ꍇ�̂�
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