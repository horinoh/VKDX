#include "DXExt.h"

void DXExt::CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs)
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
	constexpr D3D12_RASTERIZER_DESC RD = {
		.FillMode = D3D12_FILL_MODE_SOLID,
		.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
		.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, 
		.DepthClipEnable = TRUE,
		.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
		.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
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

	PipelineStates.emplace_back();
	std::vector<std::thread> Threads;
	//!< メンバ関数をスレッドで使用したい場合は、以下のようにthisを引数に取る形式を使用すればよい
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSB, NullSB, NullSB, IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), PTT, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullSB, NullSB, NullSB, IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs)
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
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK, 
		.FrontFace = DSOD, .BackFace = DSOD 
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	PipelineStates.emplace_back();
	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_MsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs)
{
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
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = DepthEnable, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = DSOD, .BackFace = DSOD
	};
	constexpr std::array RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	MESH_SHADER_PIPELINE_STATE_DESC MSPSD = {
		.pRootSignature = COM_PTR_GET(RootSignatures[0]),
		.AS = NullSB,
		.MS = SBCs[0],
		.PS = SBCs[1],
		.BlendState = D3D12_BLEND_DESC({.AlphaToCoverageEnable = TRUE, .IndependentBlendEnable = FALSE, .RenderTarget = {}}),
		.SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
		.RasterizerState = RD,
		.DepthStencilState = DSD,
		.PrimitiveTopologyType = PTT,
		.NumRenderTargets = static_cast<UINT>(size(RTVs)), .RTVFormats = {},
		.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.NodeMask = 0,
		.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0 }),
#if defined(_DEBUG) && defined(USE_WARP)
		.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG
#else
		.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
#endif
	};
	std::ranges::copy(RTVs, MSPSD.RTVFormats);

	const D3D12_PIPELINE_STATE_STREAM_DESC PSSD = { .SizeInBytes = sizeof(MSPSD), .pPipelineStateSubobjectStream = &MSPSD };
	COM_PTR<ID3D12Device2> Device2;
	VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device2)));
	VERIFY_SUCCEEDED(Device2->CreatePipelineState(&PSSD, COM_PTR_UUIDOF_PUTVOID(PipelineStates.emplace_back())));
}
