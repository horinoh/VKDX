#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	virtual void CreateShader_VsPs();
	virtual void CreateShader_VsPsDsHsGs();
	virtual void CreateShader_Cs();

	virtual void CreateRootSignature_1ConstantBuffer(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL);

	virtual void CreateInputLayout_Position();
	virtual void CreateInputLayout_PositionColor();

	virtual void CreateGraphicsPipelineState_VsPs();
	virtual void CreateGraphicsPipelineState_VsPsDsHsGs();

	template<typename T>
	void CreateConstantBuffer(const T& Type) {
		const auto Size = RoundUpTo256(sizeof(T));
		CreateUploadBuffer(ConstantBufferResource.GetAddressOf(), Size, &Type);
		CreateConstantBufferDescriptorHeap(static_cast<UINT>(Size));
#ifdef _DEBUG
		std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}

	virtual void Clear(ID3D12GraphicsCommandList* GraphicsCommandList) override { Clear_Color(GraphicsCommandList); }
	virtual void Clear_Color(ID3D12GraphicsCommandList* GraphicsCommandList);
	virtual void Clear_Depth(ID3D12GraphicsCommandList* GraphicsCommandList);
};
