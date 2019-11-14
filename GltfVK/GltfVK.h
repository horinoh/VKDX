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
		return VK_INDEX_TYPE_MAX_ENUM;
	}

	static VkPrimitiveTopology ToVKTopology(const fx::gltf::Primitive::Mode MD) {
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
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const fx::gltf::Accessor& Acc) override;

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
};
#pragma endregion