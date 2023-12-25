#include "DXExt.h"

void DXExt::CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs)
{
	//!< �u�����h (Blend)
	//!< ��) 
	//!< �u�����h	: Src * A + Dst * (1 - A)	= Src:D3D12_BLEND_SRC_ALPHA, Dst:D3D12_BLEND_INV_SRC_ALPHA, Op:D3D12_BLEND_OP_ADD
	//!< ���Z		: Src * 1 + Dst * 1			= Src:D3D12_BLEND_ONE, Dst:D3D12_BLEND_ONE, Op:D3D12_BLEND_OP_ADD
	//!< ��Z		: Src * 0 + Dst * Src		= Src:D3D12_BLEND_ZERO, Dst:D3D12_BLEND_SRC_COLOR, Op:D3D12_BLEND_OP_ADD
	const std::vector RTBDs = {
		D3D12_RENDER_TARGET_BLEND_DESC({
			.BlendEnable = FALSE, .LogicOpEnable = FALSE, //!< �u�����h�L�����ǂ����A�_�����Z�L�����ǂ��� (������TRUE�ɂ͂ł��Ȃ�)
			.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD, //!< �u�����h Src(�V�K), Dst(����), Op
			.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD, //!< �A���t�@ Src(�V�K), Dst(����), Op
			.LogicOp = D3D12_LOGIC_OP_NOOP, //!< �_�����Z
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL, //!< �������ݎ��̃}�X�N�l
		}),
	};
	constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {
		.StencilFailOp = D3D12_STENCIL_OP_KEEP,			//!< �X�e���V���e�X�g���s��
		.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,	//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g���s��
		.StencilPassOp = D3D12_STENCIL_OP_KEEP,			//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g������
		.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS		//!< �����̃X�e���V���l�Ƃ̔�r���@
	};
	//!< (�A���t�@�u�����h����)�u�e�X�g�v�͗L�������u���C�g�v�͖����ɂ���悤�ȏꍇ�� D3D12_DEPTH_WRITE_MASK_ZERO �ɂ���
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = DSOD, .BackFace = DSOD
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	std::vector<std::thread> Threads;
	//!< �����o�֐����X���b�h�Ŏg�p�������ꍇ�́A�ȉ��̂悤��this�������Ɏ��`�����g�p����΂悢
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs)
{
	const std::vector RTBDs = {
		D3D12_RENDER_TARGET_BLEND_DESC({
			.BlendEnable = FALSE, .LogicOpEnable = FALSE, 
			.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD, 
			.LogicOp = D3D12_LOGIC_OP_NOOP,
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		}),
	};
	constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {
		.StencilFailOp = D3D12_STENCIL_OP_KEEP, 
		.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, 
		.StencilPassOp = D3D12_STENCIL_OP_KEEP, 
		.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS 
	};
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK, 
		.FrontFace = DSOD, .BackFace = DSOD 
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), std::filesystem::path(".") / (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_AsMsPs(const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs)
{
	const std::vector RTBDs = {
	D3D12_RENDER_TARGET_BLEND_DESC({
		.BlendEnable = FALSE, .LogicOpEnable = FALSE,
		.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
		.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
		.LogicOp = D3D12_LOGIC_OP_NOOP,
		.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
	}),
	};
	constexpr D3D12_RASTERIZER_DESC RD = {
		.FillMode = D3D12_FILL_MODE_SOLID,
		.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
		.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		.DepthClipEnable = TRUE,
		.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
		.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {
		.StencilFailOp = D3D12_STENCIL_OP_KEEP,
		.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
		.StencilPassOp = D3D12_STENCIL_OP_KEEP,
		.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
	};
	const D3D12_DEPTH_STENCIL_DESC1 DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = DSOD, .BackFace = DSOD,
		.DepthBoundsTestEnable = FALSE,
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), std::filesystem::path(".") / (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateAsMsPs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateAsMsPs, std::ref(PipelineStates.emplace_back()), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}
