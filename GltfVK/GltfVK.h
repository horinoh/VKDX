#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Gltf.h"

class GltfVK : public VKExt, public Gltf
{
private:
	using Super = VKExt;
public:
	GltfVK() : Super() {}
	virtual ~GltfVK() {}

	static VkIndexType ToVKIndexType(const fx::gltf::Accessor::ComponentType CT) {
		switch (CT) {
		case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_INDEX_TYPE_UINT16;
		case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_INDEX_TYPE_UINT32;
		}
		DEBUG_BREAK();
		return VK_INDEX_TYPE_MAX_ENUM;
	}

	static VkPrimitiveTopology ToVKPrimitiveTopology(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
		case fx::gltf::Primitive::Mode::Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case fx::gltf::Primitive::Mode::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			//case fx::gltf::Primitive::Mode::LineLoop:
		case fx::gltf::Primitive::Mode::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case fx::gltf::Primitive::Mode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case fx::gltf::Primitive::Mode::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case fx::gltf::Primitive::Mode::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}
		DEBUG_BREAK();
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

	static VkFormat ToVKFormat(const fx::gltf::Accessor& Acc) {
		switch (Acc.type) {
		//case fx::gltf::Accessor::Type::None:
		case fx::gltf::Accessor::Type::Scalar:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec2:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec3:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8B8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8B8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16B16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16B16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32B32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32B32_SFLOAT;
			}
		case fx::gltf::Accessor::Type::Vec4:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return VK_FORMAT_R8G8B8A8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return VK_FORMAT_R8G8B8A8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return VK_FORMAT_R16G16B16A16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return VK_FORMAT_R16G16B16A16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return VK_FORMAT_R32G32B32A32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		//case fx::gltf::Accessor::Type::Mat2:
		//case fx::gltf::Accessor::Type::Mat3:
		//case fx::gltf::Accessor::Type::Mat4:
		}
		DEBUG_BREAK();
		return VK_FORMAT_UNDEFINED;
	}

protected:
	virtual void LoadScene() override;
	virtual void PushNode() override { Gltf::PushNode(); CurrentMatrix.push_back(CurrentMatrix.back()); }
	virtual void PopNode() override { Gltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) override;
	virtual void Process(const fx::gltf::Mesh& Msh) override;
	virtual void Process(const fx::gltf::Skin& Skn) override;

	virtual void Process(const fx::gltf::Material::Texture& Tex) override;
	virtual void Process(const fx::gltf::Texture& Tex) override;

	virtual std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) override { 
		const auto v = glm::mix(*reinterpret_cast<const glm::vec3*>(lhs.data()), *reinterpret_cast<const glm::vec3*>(rhs.data()), t);
		return { v.x, v.y, v.z };
	}
	virtual std::array<float, 4> SLerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) override { 
		const auto q = glm::slerp(*reinterpret_cast<const glm::quat*>(lhs.data()), *reinterpret_cast<const glm::quat*>(rhs.data()), t);
		return { q.x, q.y, q.z, q.w };
	}

	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

	std::vector<glm::mat4> CurrentMatrix = { glm::identity<glm::mat4>() };
	
	float CurrentFrame = 0.0f;
	
	std::vector<const glm::mat4*> InverseBindMatrices;
	std::vector<glm::mat4> JointMatrices;

	std::vector<float> MorphWeights;
};
#pragma endregion