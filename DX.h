#pragma once

#define USE_WINRT
#define USE_WRL
#ifdef USE_WINRT
//!< �v���W�F�N�g�E�N���b�N - Property - All Configurations �ɂ��� - C / C++ - Language - C++ Language Standard - ISO C++17 Standard ��I�����Ă������� (�f�t�H���g�ł�C++14)
#include <winrt/base.h>
#endif
#ifdef USE_WRL
#include <wrl.h>
#endif

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_4.h>

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

//!< _DEBUG �ł���Ή������Ȃ��Ă� PIX �g�p�\�ARelease �� PIX ���g�p�������悤�ȏꍇ�� USE_PIX ���`���� 
//!< When want to use pix in Release build, define USE_PIX
//!< �\�����[�V�����E�N���b�N - �\�����[�V������NuGet�p�b�P�[�W�̊Ǘ� - �Q�ƃ^�u - WinPixEventRuntime�Ō��� - �v���W�F�N�g��I�����ăC���X�g�[�����Ă�������
//#define USE_PIX
#include <pix3.h>
//!< �v���O��������L���v�`�����s�������ꍇ Capture in program code
#if defined(_DEBUG) || defined(USE_PIX)
#include <DXProgrammableCapture.h>
#endif

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(FAILED(vr)) { DEBUG_BREAK(); }
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
	//virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { Super::OnTimer(hWnd, hInstance); }
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result) { return ToString(GetHRESULTWString(Result)); }
	static std::wstring GetHRESULTWString(const HRESULT Result) { return std::wstring(_com_error(Result).ErrorMessage()); }
	static std::string GetFormatString(const DXGI_FORMAT Format);

protected:
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source);
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes);

	virtual void ExecuteCopyBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const size_t Size);
	virtual void ExecuteCopyTexture(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);

	virtual void CreateUploadResource(ID3D12Resource** Resource, const size_t Size);
	virtual void CreateDefaultResource(ID3D12Resource** Resource, const size_t Size);

	virtual void ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);
	virtual void PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES ResourceState);
	
#if defined(_DEBUG) || defined(USE_PIX)
	//!< #DX_TODO PIX �֘A
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName(ID3D12DeviceChild* Resource, LPCWSTR Name) { Resource->SetName(Name); }
	static void SetName(ID3D12DeviceChild* Resource, const std::wstring& Name) { SetName(Resource, Name.c_str()); }
	static void SetName(ID3D12DeviceChild* Resource, const std::string& Name) { SetName(Resource, ToWString(Name)); }
#endif

	virtual void CreateDevice(HWND hWnd);
	virtual HRESULT CreateMaxFeatureLevelDevice(IDXGIAdapter* Adapter);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel();
	virtual void CheckMultiSample(const DXGI_FORMAT Format);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index = 0) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index = 0) const;
	
	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const size_t Count, const D3D12_COMMAND_LIST_TYPE CommandListType);
	virtual void CreateCommandList();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const RECT& Rect) { CreateSwapChain(hWnd, ColorFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void CreateSwapChainResource();
	virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void InitializeSwapChain();
	virtual void ResetSwapChainResource() { 
		for (auto& i : SwapChainResources) {
#ifdef USE_WINRT
			i = nullptr;
#elif defined(USE_WRL)
			i.Reset();
#endif
		} 
	}
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeSwapChain(const RECT& Rect) { ResizeSwapChain(static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	UINT AcquireNextBackBufferIndex() const {
		return 0xffffffff == CurrentBackBufferIndex ? SwapChain->GetCurrentBackBufferIndex() : (CurrentBackBufferIndex + 1) % static_cast<const UINT>(SwapChainResources.size());
	}

	virtual void CreateDepthStencil() {}
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rect) { CreateDepthStencil(DepthFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void CreateDepthStencilResource(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rect) { ResizeDepthStencil(DepthFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }

	virtual void LoadImage(ID3D12Resource** Resource/*, ID3D12DescriptorHeap** DescriptorHeap*/, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { assert(false && "Not implemanted"); }
	virtual void LoadImage(ID3D12Resource** Resource/*, ID3D12DescriptorHeap** DescriptorHeap*/, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource/*, DescriptorHeap*/, ToWString(Path), ResourceState); }
	
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer(ID3D12Resource** Resource, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL);
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer();

	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewport(const RECT& Rect, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) { CreateViewport(static_cast<FLOAT>(Rect.right - Rect.left), static_cast<FLOAT>(Rect.bottom - Rect.top), MinDepth, MaxDepth); }
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }
	
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {}
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {}
	virtual void CreateRootSignature();

	virtual void CreateDescriptorHeap() {}
	virtual void UpdateDescriptorHeap() { /*CopyToUploadResource()�����s��*/ }

#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const;
	virtual void CreateShaderByteCode(const std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBCs) const; 
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	virtual void CreateShaderByteCode(const std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBCs) const; 
#endif	
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs) const {}
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const = 0; //!< D3D12_GRAPHICS_PIPELINE_STATE_DESC �쐬���Ɏg�p
	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const = 0; //!< IASetPrimitiveTopology() ���Ɏg�p
	virtual BOOL LoadPipelineLibrary(const std::wstring& Path);
	virtual void StorePipelineLibrary(const std::wstring& Path) const;
	virtual void CreatePipelineState();
	virtual void CreatePipelineState_Graphics();
	virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateStaticSamplerDesc(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const {}

	virtual void ClearColor(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color);
	virtual void ClearDepthStencil(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const FLOAT Depth = 1.0f, const UINT8 Stencil = 0);
	virtual void PopulateCommandList(const size_t i);

	virtual void Draw();
	virtual void Dispatch();
	virtual void Present();
	virtual void WaitForFence();

protected:
#if defined(_DEBUG) || defined(USE_PIX)
#ifdef USE_WINRT
	winrt::com_ptr<IDXGraphicsAnalysis> GraphicsAnalysis;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Device> Device;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
#endif
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12CommandQueue> CommandQueue;
	winrt::com_ptr<ID3D12Fence> Fence;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
#endif
	UINT64 FenceValue = 0;

#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	//std::vector<winrt::com_ptr<ID3D12CommandList>> CommandLists;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12CommandList>> CommandLists;
#endif
	
#ifdef USE_WINRT
	winrt::com_ptr<IDXGISwapChain3> SwapChain;
	winrt::com_ptr<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	std::vector<winrt::com_ptr<ID3D12Resource>> SwapChainResources;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SwapChainDescriptorHeap; 
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> SwapChainResources;
#endif
	UINT CurrentBackBufferIndex = 0xffffffff;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> DepthStencilResource;
	winrt::com_ptr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap; 
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> ImageResource;
	winrt::com_ptr<ID3D12DescriptorHeap> ImageDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> ImageResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ImageDescriptorHeap;
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> SamplerResource;
	winrt::com_ptr<ID3D12DescriptorHeap> SamplerDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> SamplerResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap; 
#endif
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12RootSignature> RootSignature;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12PipelineLibrary> PipelineLibrary;
	winrt::com_ptr<ID3D12PipelineState> PipelineState; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> PipelineLibrary;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState; 
#endif
	
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> VertexBufferResource;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
#endif
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> IndexBufferResource;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
#endif
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> IndirectBufferResource;
	winrt::com_ptr<ID3D12CommandSignature> IndirectCommandSignature; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> IndirectBufferResource;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> IndirectCommandSignature; 
#endif

	//!< ����1�̂݁A�z��ɂ��� #DX_TODO
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> ConstantBufferResource;
	winrt::com_ptr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap; 
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> UnorderedAccessTextureResource;
	winrt::com_ptr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> UnorderedAccessTextureResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
#endif
	
	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

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
