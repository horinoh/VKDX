#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreateVertexInputBinding<Vertex_Position>(std::vector<VkVertexInputBindingDescription>& BindDescs, std::vector<VkVertexInputAttributeDescription>& AttrDescs, const uint32_t Binding) const {
	BindDescs.push_back({ Binding, sizeof(Vertex_Position), VK_VERTEX_INPUT_RATE_VERTEX }); //!< バーテックス毎(インスタンス毎にする場合には VK_VERTEX_INPUT_RATE_INSTANCE を使用する)

	AttrDescs.push_back({ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_Position, Position) }); //!< layout (location = 0) in vec3 InPosition
}
template<> void CreateVertexInputBinding<Vertex_PositionColor>(std::vector<VkVertexInputBindingDescription>& BindDescs, std::vector<VkVertexInputAttributeDescription>& AttrDescs, const uint32_t Binding) const {
	BindDescs.push_back({ Binding, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX });

	AttrDescs.push_back({ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) }); //!< layout(location = 0) in vec3 InPosition
	AttrDescs.push_back({ 1, Binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex_PositionColor, Color) }); //!< layout(location = 1) in vec4 InColor
}