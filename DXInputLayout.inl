#pragma once

//!< �e���v���[�g���ꉻ
//!< template specialization

template<> void CreateInputLayoutSlot<Vertex_Position>(std::vector<D3D12_INPUT_ELEMENT_DESC>& InElemDescs, const UINT Slot) const {
	//!< �o�[�e�b�N�X��(�C���X�^���X���ɂ���ꍇ�ɂ� D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA ���g�p����)
	InElemDescs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, Slot, offsetof(Vertex_Position, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }); //!< float3 Position : POSITION
}

template<> void CreateInputLayoutSlot<Vertex_PositionColor>(std::vector<D3D12_INPUT_ELEMENT_DESC>& InElemDescs, const UINT Slot) const {
	InElemDescs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, Slot, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }); //!< float3 Position : POSITION
	InElemDescs.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, Slot, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }); //!< float4 Color : COLOR
}