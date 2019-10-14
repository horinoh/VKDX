#pragma once

#include "resource.h"

#pragma region Code
#include "../VKImage.h"

class TextureVK : public VKImage
{
private:
	using Super = VKImage;
public:
	TextureVK() : Super() {}
	virtual ~TextureVK() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
#ifdef USE_IMMUTABLE_SAMPLER
		assert(!Samplers.empty() && "");
		//!< ImmutableSampler(DX �� STATIC_SAMPLER_DESC �����Ǝv����)���g���ꍇ 
		//!< �Z�b�g���C�A�E�g�ɉi���I�Ƀo�C���h����ύX�ł��Ȃ�
		//!< �R���o�C���h�C���[�W�T���v���[�̕ύX�̏ꍇ�́A�C���[�W�r���[�ւ̕ύX�͔��f����邪�T���v���ւ̕ύX�͖��������
		const std::array<VkSampler, 1> ISs = { 
			Samplers[0] 
		};
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
			});
#else
		//!< �ʏ�̃T���v�����g���ꍇ
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0],
			{
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			});
#endif
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts[0] 
			}, {});
	}

#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
	virtual void CreateDescriptorUpdateTemplate() override {
		const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos), sizeof(DescriptorUpdateInfo)
			}
		};

		assert(!DescriptorSetLayouts.empty() && "");
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(DUTEs.size()), DUTEs.data(),
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			DescriptorSetLayouts[0],
			VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0
		};
		DescriptorUpdateTemplates.resize(1);
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));
	}
#endif

	virtual void CreateDescriptorPool() override { 
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 } 
			});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorSetLayouts.empty() && "");
		const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
		assert(!DescriptorPools.empty() && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPools[0],
			static_cast<uint32_t>(DSLs.size()), DSLs.data()
		};
		DescriptorSets.resize(1);
		for (auto& i : DescriptorSets) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &i));
		}
	}
	virtual void UpdateDescriptorSet() override {
#ifdef USE_IMMUTABLE_SAMPLER
		assert(!Samplers.empty() && "");
#endif
		assert(VK_NULL_HANDLE != ImageView && "");
		const DescriptorUpdateInfo DUI = {
#ifdef USE_IMMUTABLE_SAMPLER
			{ VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
#else
			{ Samplers[0], ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
#endif
		};

		assert(!DescriptorSets.empty() && "");
#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
#else
		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 0, 0,
					_countof(DescriptorUpdateInfo::DescriptorImageInfos), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DUI.DescriptorImageInfos, nullptr, nullptr
				}
			},
			{});
#endif
	}

	virtual void CreateTexture() override {
#if 1
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "UV.dds");
#else
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_a8_unorm.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds"); //!< #VK_TODO ... gli���ł�����
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgr8_unorm.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgra8_srgb.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgra8_unorm.dds"); //!< OK
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgrx8_srgb.dds"); //!< OK 
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgrx8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_l8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r_ati1n_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r5g6b5_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r8_sint.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r8_uint.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r16_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rg_ati2n_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rg11b10_ufloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_atc_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc2_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc2_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_pvrtc_2bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_pvrtc_4bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb5a1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb5a1_unorm_.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb9e5_ufloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb10a2_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb10a2u.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_astc4x4_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_astc8x8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_atc_explicit_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_atc_interpolate_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt1_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm1.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm2.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_pvrtc2_4bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba4_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_snorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba16_sfloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_bgr8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_rgba_dxt1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_rgba8_srgb.dds");
#endif
	}

	//!< VK�̏ꍇ�C�~���[�^�u���T���v���ƒʏ�̃T���v���͊�{�I�ɓ������́A�f�X�N���v�^�Z�b�g���C�A�E�g�̎w�肪�قȂ邾��
	virtual void CreateSampler() override {
		Samplers.resize(1);
#ifdef USE_IMMUTABLE_SAMPLER
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, // min, mag, mip
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, // u, v, w
			0.0f, // lod bias
			VK_FALSE, 1.0f, // anisotropy
			VK_FALSE, VK_COMPARE_OP_NEVER, // compare
			0.0f, 1.0f, // min, maxlod
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, // border
			VK_FALSE // addressing VK_FALSE:���K��[0.0-1.0], VK_TRUE:�摜�̃f�B�����V����
		};
#else
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, //!< ��C�~���[�^�u���T���v���̏ꍇ������ NEAREST �ɂ��Ă���
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
#endif
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { Pipelines.resize(1);  CreatePipeline_VsFs(Pipelines[0]); }
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DescriptorImageInfos[1];
	};
};
#pragma endregion