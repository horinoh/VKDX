#pragma once

//!< ���Ȃ��Ƃ� USE_WINRT, USE_WRL �̂����ꂩ�͒�`���邱�ƁA������`���ꂽ�ꍇ�� USE_WINRT ���D�悳��� (At least define USE_WINRT or USE_WRL, if both defined USE_WINRT will be used)
#define USE_WINRT
#define USE_WRL
#ifdef USE_WINRT
//!< Property - All Configurations, C/C++ - Language - C++ Language Standard - Select ISO C++17 Standard (Default is C++14)
#include <winrt/base.h>
#define COM_PTR winrt::com_ptr
#define COM_PTR_GET(_x) _x.get()
#define COM_PTR_PUT(_x) _x.put()
#define COM_PTR_PUTVOID(_x) _x.put_void()
#define COM_PTR_UUIDOF_PUTVOID(_x) __uuidof(_x), COM_PTR_PUTVOID(_x)
#define COM_PTR_RESET(_x) _x = nullptr
#define COM_PTR_AS(_x, _y) winrt::copy_to_abi(_x, *_y.put_void());
#elif defined(USE_WRL)
#include <wrl.h>
#define COM_PTR Microsoft::WRL::ComPtr
#define COM_PTR_GET(_x) _x.Get()
#define COM_PTR_PUT(_x) _x.GetAddressOf()
#define COM_PTR_PUTVOID(_x) _x.GetAddressOf()
#define COM_PTR_UUIDOF_PUTVOID(_x) IID_PPV_ARGS(COM_PTR_PUTVOID(_x))
#define COM_PTR_RESET(_x) _x.Reset()
#define COM_PTR_AS(_x, _y) VERIFY_SUCCEEDED(_x.As(&_y));
#endif

//#define USE_WARP
#define USE_STATIC_SAMPLER //!< TextureDX
//!< HLSL���烋�[�g�V�O�l�`�����쐬���� (Create root signature from HLSL)
//#define USE_HLSL_ROOTSIGNATRUE
//!< �o���h�� : VK�̃Z�J���_���R�}���h�o�b�t�@����
#define USE_BUNDLE //!< ParametricSurfaceDX
//!< ���[�g�R���X�^���g : VK�̃v�b�V���R���X�^���g����
//#define USE_ROOT_CONSTANTS //!< TriangleDX
//#define USE_GAMMA_CORRECTION

#include <initguid.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
//!< DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h �ɒ�`�����݂���̂������ֈڐA
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8  | (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24)
#include <DXGI1_6.h>

#include <DirectXMath.h>
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

//!< _DEBUG �ł���Ή������Ȃ��Ă� PIX �g�p�\�ARelease �� PIX ���g�p�������悤�ȏꍇ�� USE_PIX ���`���� (When want to use pix in Release build, define USE_PIX)
//!< �\�����[�V�����E�N���b�N - �\�����[�V������NuGet�p�b�P�[�W�̊Ǘ� - �Q�ƃ^�u - WinPixEventRuntime�Ō��� - �v���W�F�N�g��I�����ăC���X�g�[�����Ă�������
//#define USE_PIX
#include <pix3.h>
//!< �v���O��������L���v�`�����s�������ꍇ (Capture in program code)
#if defined(_DEBUG) || defined(USE_PIX)
#include <DXProgrammableCapture.h>
#endif

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(FAILED(vr)) { Log(DX::GetHRESULTString(vr).c_str()); DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + DX::GetHRESULTString(hr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(hr) if(FAILED(hr)) { Win::ShowMessageBoxW(nullptr, DX::GetHRESULTStringW(hr)); }
#endif

#include "Cmn.h"
#include "Win.h"

/**
���\�[�X���쐬���ꂽ�� MakeResident() ����A�j�����ꂽ�� Evict() �����B
�A�v�����疾���I�ɂ�����s�������ꍇ�͈ȉ��̂悤�ɂ���
ID3D12Resource* Resource;
const std::vector<ID3D12Pageable*> Pageables = { Resource };
Device->MakeResident(static_cast<UINT>(Pageables.size()), Pageables.data());
Device->Evict(static_cast<UINT>(Pageables.size()), Pageables.data());
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
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	//virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override {}
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnTimer(hWnd, hInstance); 
	}
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnPaint(hWnd, hInstance); 
		Draw();
	}
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result) { return ToString(GetHRESULTWString(Result)); }
	static std::wstring GetHRESULTWString(const HRESULT Result) { return std::wstring(_com_error(Result).ErrorMessage()); }
	static std::string GetFormatString(const DXGI_FORMAT Format);

	static std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT3(lhs.data());
		const auto r = DirectX::XMFLOAT3(rhs.data());
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&l), DirectX::XMLoadFloat3(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2] };
	}
	static std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT4(lhs.data());
		const auto r = DirectX::XMFLOAT4(rhs.data());
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&l), DirectX::XMLoadFloat4(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2], v.m128_f32[3] };
	}

protected:
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range = nullptr);
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData);

	virtual void ExecuteCopyBuffer(ID3D12Resource* DstResource, ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, const size_t Size, ID3D12Resource* SrcResource);
	virtual void ExecuteCopyTexture(ID3D12Resource* DstResource, ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState, ID3D12Resource* SrcResource);

	virtual void CreateUploadResource(ID3D12Resource** Resource, const size_t Size);
	virtual void CreateDefaultResource(ID3D12Resource** Resource, const size_t Size);

	virtual void ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);
	virtual void PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES RS);

#if defined(_DEBUG) || defined(USE_PIX)
	//!< #DX_TODO PIX �֘A
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName(ID3D12DeviceChild * Resource, LPCWSTR Name) { Resource->SetName(Name); }
	static void SetName(ID3D12DeviceChild* Resource, const std::wstring& Name) { SetName(Resource, Name.c_str()); }
	static void SetName(ID3D12DeviceChild* Resource, const std::string& Name) { SetName(Resource, ToWString(Name)); }
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

	virtual void CreateCommandAllocator();
	virtual void CreateCommandList();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void CreateSwapChainResource();
	virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void ResetSwapChainResource() {
		for (auto& i : SwapChainResources) {
			COM_PTR_RESET(i);
		}
		SwapChainResources.clear();
	}
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);

	virtual void CreateRenderTarget() {}
	//virtual void CreateRenderTarget(const DXGI_FORMAT Format, const UINT Width, const UINT Height);

	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);

	virtual void LoadImage(ID3D12Resource** /*Resource*/, const std::wstring& /*Path*/, const D3D12_RESOURCE_STATES /*ResourceState*/) { assert(false && "Not implemanted"); }
	virtual void LoadImage(ID3D12Resource** Resource, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource/*, DescriptorHeap*/, ToWString(Path), ResourceState); }

	virtual void LoadScene() {}

	virtual void CreateAndCopyToUploadResource(COM_PTR<ID3D12Resource>& Res, const size_t Size, const void* Source);
	virtual void CreateAndCopyToDefaultResource(COM_PTR<ID3D12Resource>& Res, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL, const size_t Size, const void* Source);
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer() {}

	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);

	virtual void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path);
	virtual void CreateRootSignature();

	virtual void CreateDescriptorHeap() {}
	virtual void CreateDescriptorView() {}

	virtual void CreateShaderBlobs() {}

#include "DXPipelineLibrary.inl"
	virtual void CreatePipelineStates() {}
	static void CreatePipelineState(COM_PTR<ID3D12PipelineState>& PST, ID3D12Device* Device, ID3D12RootSignature* RS,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology,
		const D3D12_RASTERIZER_DESC& RD,
		const D3D12_DEPTH_STENCIL_DESC& DSD,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, 
		const std::vector<DXGI_FORMAT>& RtvFormats,
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);
	//virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateStaticSampler() {}
	virtual void CreateSampler() {}

	virtual void PopulateCommandList(const size_t i);

	virtual void Draw();
	virtual void Dispatch();
	virtual void Present();
	virtual void WaitForFence();

protected:
#if defined(_DEBUG) || defined(USE_PIX)
	COM_PTR<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif

	COM_PTR<IDXGIFactory4> Factory;
	COM_PTR<IDXGIAdapter> Adapter;
	COM_PTR<IDXGIOutput> Output;

	COM_PTR<ID3D12Device> Device;
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
	COM_PTR<ID3D12CommandQueue> CommandQueue;
	COM_PTR<ID3D12Fence> Fence;
	UINT64 FenceValue = 0;

	std::vector<COM_PTR<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	std::vector<COM_PTR<ID3D12CommandAllocator>> BundleCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> BundleGraphicsCommandLists;
	//std::vector<COM_PTR<ID3D12CommandList>> CommandLists;
	
	COM_PTR<IDXGISwapChain4> SwapChain;
	std::vector<COM_PTR<ID3D12Resource>> SwapChainResources;

	COM_PTR<ID3D12Resource> UnorderedAccessTextureResource;
	std::vector<COM_PTR<ID3D12Resource>> ImageResources;
	std::vector<COM_PTR<ID3D12Resource>> ConstantBufferResources;
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap; //!< RTV�����ǁA����X���b�v�`�F�C�������ʈ����ɂ��Ă��� #DX_TODO
	std::vector<COM_PTR<ID3D12DescriptorHeap>> SamplerDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> RtvDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> DsvDescriptorHeaps;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> CbvSrvUavDescriptorHeaps;

	std::vector<COM_PTR<ID3D12RootSignature>> RootSignatures;

	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	std::vector<COM_PTR<ID3D12PipelineState>> PipelineStates;

	std::vector<COM_PTR<ID3D12Resource>> VertexBufferResources;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

	std::vector <COM_PTR<ID3D12Resource>> IndexBufferResources;
	std::vector<D3D12_INDEX_BUFFER_VIEW> IndexBufferViews;

	std::vector<COM_PTR<ID3D12Resource>> IndirectBufferResources;
	std::vector<COM_PTR<ID3D12CommandSignature>> IndirectCommandSignatures;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;

	std::array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> DescriptorHandleIndices{};

protected:
	const std::vector<D3D_FEATURE_LEVEL> FeatureLevels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	const D3D12_SHADER_BYTECODE NullShaderBC = { nullptr, 0 };
};

#ifdef DEBUG_STDOUT
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMVECTOR& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.m128_f32[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMMATRIX& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.r[i]; } rhs << std::endl; return rhs; }
#endif