#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); } //!< 最低でもインデックス数1が必要 At least index count must be 1

#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const override {
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> & ShaderBlobs) const override {
#endif
		CreateShader_VsPsDsHsGs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}

#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob) override;
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob) override;
#endif

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override {
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override {
		const UINT PatchControlPoint = 1;
		return static_cast<D3D_PRIMITIVE_TOPOLOGY>(static_cast<UINT>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + (PatchControlPoint - 1));
	}
	
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion