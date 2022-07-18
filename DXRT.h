#pragma once

#include "DXExt.h"

class DXRT : public DXExt
{
private:
	using Super = DXExt;
protected:
	virtual void CreateDevice(HWND hWnd) override {
		Super::CreateDevice(hWnd);

		//!< ID3D12Device5 はよく使うので覚えておく
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));
	}

	virtual void CreateTexture() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		UnorderedAccessTextures.emplace_back().Create(COM_PTR_GET(Device), GetClientRectWidth(), GetClientRectHeight(), 1, SCD.Format);
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
		constexpr std::array DRs_Tlas = {
			//!< TLAS (SRV0) : register(t0, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		constexpr std::array DRs_Uav = {
			//!< 出力 (UAV0) : register(u0, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			//!< TLAS (SRV0)
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Tlas)), .pDescriptorRanges = data(DRs_Tlas) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
			//!< 出力 (UAV0)
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));

		CbvSrvUavGPUHandles.emplace_back();
		auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
		auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//!< [0] TLAS (SRV0)
		Device->CreateShaderResourceView(nullptr/* AS の場合 nullptr を指定する*/, &TLASs[0].SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [1] 出力 (UAV0)
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
	}

	[[nodiscard]] static bool HasRaytracingSupport(ID3D12Device* Dev) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 FDO5;
		return SUCCEEDED(Dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, reinterpret_cast<void*>(&FDO5), sizeof(FDO5))) && D3D12_RAYTRACING_TIER_NOT_SUPPORTED != FDO5.RaytracingTier;
	}

	class ASCompaction
	{
	public:
		COM_PTR<ID3D12Resource> Info;
		COM_PTR<ID3D12Resource> Read;
		virtual ASCompaction& Create(ID3D12Device* Dev) {
			constexpr auto Size = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC);
			DX::CreateBufferResource(COM_PTR_PUT(Info), Dev, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			DX::CreateBufferResource(COM_PTR_PUT(Read), Dev, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST);
			return *this;
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL) {
			//!< リードバックへのコピーコマンドを発行する 
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
			//!< 結果をリードバックへコピーした後に使用すること
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
#define TO_GCL4(GCL, GCL4) COM_PTR<ID3D12GraphicsCommandList4> GCL4; VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)))
	public:
		virtual AccelerationStructureBuffer& Create(ID3D12Device* Dev, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Dev, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
			return *this;
		}
		void PopulateBuildCommand(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD, ID3D12GraphicsCommandList* GCL, ASCompaction* Compaction = nullptr) {
			TO_GCL4(GCL, GCL4);
			if (nullptr != Compaction) {
				//!< コンパクションサイズを取得する設定
				const std::array RASPIDs = {
					D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC({.DestBuffer = Compaction->Info->GetGPUVirtualAddress(), .InfoType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE}),
				};
				GCL4->BuildRaytracingAccelerationStructure(&BRASD, static_cast<UINT>(size(RASPIDs)), data(RASPIDs));

				//!< 結果をリードバックへコピー
				Compaction->PopulateCopyCommand(GCL);
			}
			else {
				constexpr std::array<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC, 0> RASPIDs = {};
				GCL4->BuildRaytracingAccelerationStructure(&BRASD, static_cast<UINT>(size(RASPIDs)), data(RASPIDs));
			}
		}
		void PopulateBuildCommand(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12Resource* Scratch, ASCompaction* Compaction = nullptr) {
			const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD = {
				.DestAccelerationStructureData = Resource->GetGPUVirtualAddress(),
				.Inputs = BRASI,
				.SourceAccelerationStructureData = 0,
				.ScratchAccelerationStructureData = Scratch->GetGPUVirtualAddress()
			};
			PopulateBuildCommand(BRASD, GCL, Compaction);
		}
		void ExecuteBuildCommand(ID3D12Device* Dev, const size_t Size, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, ASCompaction* Compaction = nullptr) {
			ScratchBuffer SB;
			SB.Create(Dev, Size);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateBuildCommand(BRASI, GCL, COM_PTR_GET(SB.Resource), Compaction);
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, static_cast<ID3D12CommandList*>(GCL), Fence);
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
	};
	class BLAS : public AccelerationStructureBuffer
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		virtual BLAS& Create(ID3D12Device* Dev, const size_t Size) {
			Super::Create(Dev, Size);
			return *this;
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src) {
			TO_GCL4(GCL, GCL4);
			GCL4->CopyRaytracingAccelerationStructure(Resource->GetGPUVirtualAddress(), Src->GetGPUVirtualAddress(), D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT);
		}
		void ExecuteCopyCommand(ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, ID3D12Resource* Src) {
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, Src);
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
		virtual TLAS& Create(ID3D12Device* Dev, const size_t Size) override {
			Super::Create(Dev, Size);
			SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.RaytracingAccelerationStructure = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV({.Location = Resource->GetGPUVirtualAddress()})
				});
			return *this;
		}
	};
	//class UploadStructuredBuffer : public UploadResource
	//{
	//private:
	//	using Super = UploadResource;
	//public:
	//	D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
	//	UploadStructuredBuffer& Create(ID3D12Device* Device, const size_t Size, const size_t Stride, const void* Source) {
	//		Super::Create(Device, Size, Source);
	//		SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
	//			.Format = DXGI_FORMAT_UNKNOWN,
	//			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
	//			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	//			.Buffer = D3D12_BUFFER_SRV({
	//				.FirstElement = 0,
	//				.NumElements = static_cast<UINT>(Size / Stride),
	//				.StructureByteStride = static_cast<UINT>(Stride),
	//				.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
	//			})
	//		});
	//		return *this;
	//	}
	//};
	class ScratchBuffer : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		void Create(ID3D12Device* Dev, const size_t Size) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Dev, Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	};
	class ShaderTable : public ResourceBase
	{
	private:
		using Super = ResourceBase;
	public:
		//!< Gen 用
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE AddressRange = { .StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0 };
		//!< 順序は決まっていないが、Miss, Hit, Call 用の 3 つ分
		std::array<D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE, 3> AddressRangeAndStrides = {
			D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0 }),
			D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0 }),
			D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0 }),
		};
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE Range;
		ShaderTable& Create(ID3D12Device* Dev, const size_t Size, const size_t Stride) {
			DX::CreateBufferResource(COM_PTR_PUT(Resource), Dev, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
			Range = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({
				.StartAddress = Resource->GetGPUVirtualAddress(),
				.SizeInBytes = Size,
				.StrideInBytes = Stride //!< 個数が1つの場合は 0 でも良い
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

	std::vector<BLAS> BLASs;
	std::vector<TLAS> TLASs;
	std::vector<ShaderTable> ShaderTables;
	std::vector<COM_PTR<ID3D12StateObject>> StateObjects;

	COM_PTR<ID3D12Device5> Device5;
};
