#include "DXExt.h"

void DXExt::CreatePipelineState_VsPs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs)
{
	//!< ブレンド (Blend)
	//!< 例) 
	//!< ブレンド	: Src * A + Dst * (1 - A)	= Src:D3D12_BLEND_SRC_ALPHA, Dst:D3D12_BLEND_INV_SRC_ALPHA, Op:D3D12_BLEND_OP_ADD
	//!< 加算		: Src * 1 + Dst * 1			= Src:D3D12_BLEND_ONE, Dst:D3D12_BLEND_ONE, Op:D3D12_BLEND_OP_ADD
	//!< 乗算		: Src * 0 + Dst * Src		= Src:D3D12_BLEND_ZERO, Dst:D3D12_BLEND_SRC_COLOR, Op:D3D12_BLEND_OP_ADD
	const std::vector RTBDs = {
		D3D12_RENDER_TARGET_BLEND_DESC({
			.BlendEnable = FALSE, .LogicOpEnable = FALSE, //!< ブレンド有効かどうか、論理演算有効かどうか (同時にTRUEにはできない)
			.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD, //!< ブレンド Src(新規), Dst(既存), Op
			.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD, //!< アルファ Src(新規), Dst(既存), Op
			.LogicOp = D3D12_LOGIC_OP_NOOP, //!< 論理演算
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL, //!< 書き込み時のマスク値
		}),
	};
	constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {
		.StencilFailOp = D3D12_STENCIL_OP_KEEP,			//!< ステンシルテスト失敗時
		.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,	//!< ステンシルテスト成功、デプステスト失敗時
		.StencilPassOp = D3D12_STENCIL_OP_KEEP,			//!< ステンシルテスト成功、デプステスト成功時
		.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS		//!< 既存のステンシル値との比較方法
	};
	//!< (アルファブレンド等で)「テスト」は有効だが「ライト」は無効にするような場合は D3D12_DEPTH_WRITE_MASK_ZERO にする
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = DSOD, .BackFace = DSOD
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	//!< メンバ関数をスレッドで使用したい場合は、以下のようにthisを引数に取る形式を使用すればよい
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif	
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs)
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

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), std::filesystem::path(".") / (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, nullptr, nullptr));
#endif	
}

void DXExt::CreatePipelineState_VsPsGs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs)
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

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), std::filesystem::path(".") / (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, SBCs[2], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PST), COM_PTR_GET(Device), RS, Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSBC, NullSBC, SBCs[2], IEDs, RTVs, nullptr, nullptr));
#endif	
}

void DXExt::CreatePipelineState_AsMsPs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs)
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

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), std::filesystem::path(".") / (GetTitleString() + ".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateAsMsPs, std::ref(PST), COM_PTR_GET(Device), RS, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateAsMsPs, std::ref(PST), COM_PTR_GET(Device), RS, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], RTVs, nullptr, nullptr));
#endif	
}

#if 0
void DXExt::CreatePipelineState(COM_PTR<ID3D12PipelineState>& PST,
	ID3D12RootSignature* RS,
	D3D12_SHADER_BYTECODE VS, D3D12_SHADER_BYTECODE PS, D3D12_SHADER_BYTECODE DS, D3D12_SHADER_BYTECODE HS, D3D12_SHADER_BYTECODE GS,
	D3D12_STREAM_OUTPUT_DESC SOD,
	D3D12_BLEND_DESC BD,
	UINT SampleMask,
	D3D12_RASTERIZER_DESC RD,
	D3D12_DEPTH_STENCIL_DESC DSD,
	D3D12_INPUT_LAYOUT_DESC ILD,
	D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBSCV,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT,
	std::vector<DXGI_FORMAT>& RTVFormats,
	//UINT NumRenderTargets, DXGI_FORMAT RTVFormats[8],
	DXGI_FORMAT DSVFormat,
	DXGI_SAMPLE_DESC SD,
	UINT NodeMask,
	D3D12_CACHED_PIPELINE_STATE CPS,
	D3D12_PIPELINE_STATE_FLAGS PSF)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {
		.pRootSignature = RS,
		.VS = VS, .PS = PS, .DS = DS, .HS = HS, .GS = GS,
		.StreamOutput = SOD,
		.BlendState = BD,
		.SampleMask = SampleMask,
		.RasterizerState = RD,
		.DepthStencilState = DSD,
		.InputLayout = ILD,
		.IBStripCutValue = IBSCV,
		.PrimitiveTopologyType = PTT,
		.NumRenderTargets = static_cast<UINT>(std::size(RTVFormats)), .RTVFormats = {},//std::data(RTVFormats),
		.DSVFormat = DSVFormat,
		.SampleDesc = SD,
		.NodeMask = NodeMask,
		.CachedPSO = CPS,
#if defined(_DEBUG) && defined(USE_WARP)
		//!< パイプラインがデバッグ用付加情報ありでコンパイルされる、WARP時のみ使用可能
		.Flags = PSF | D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG
#else
		.Flags = PSF
#endif
	};
}
#endif
