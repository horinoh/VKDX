#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT VertexCount, const UINT InstanceCount)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const D3D12_DRAW_ARGUMENTS Source = { 
		.VertexCountPerInstance = VertexCount,
		.InstanceCount = InstanceCount, 
		.StartVertexLocation = 0, 
		.StartInstanceLocation = 0 
	};
	constexpr auto Stride = sizeof(Source);
	constexpr auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	constexpr std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({ .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW }), };
	constexpr D3D12_COMMAND_SIGNATURE_DESC CSD = {
		.ByteStride = Stride,
		.NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs),
		.NodeMask = 0
	};
	//!< パイプラインのバインディングを更新するような場合はルートシグネチャが必要、Draw や Dispatch のみの場合はnullptrを指定できる
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { 
		.IndexCountPerInstance = IndexCount, 
		.InstanceCount = InstanceCount, 
		.StartIndexLocation = 0, 
		.BaseVertexLocation = 0, 
		.StartInstanceLocation = 0
	};
	constexpr auto Stride = sizeof(Source);
	constexpr auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	constexpr std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({ .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED }), };
	constexpr D3D12_COMMAND_SIGNATURE_DESC CSD = {
		.ByteStride = Stride,
		.NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs),
		.NodeMask = 0
	};
	Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const D3D12_DISPATCH_ARGUMENTS Source = { 
		.ThreadGroupCountX = X, 
		.ThreadGroupCountY = Y,
		.ThreadGroupCountZ = Z 
	};
	constexpr auto Stride = sizeof(Source);
	constexpr auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	constexpr std::array IADs = { D3D12_INDIRECT_ARGUMENT_DESC({ .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH }), };
	constexpr D3D12_COMMAND_SIGNATURE_DESC CSD = {
		.ByteStride = Stride,
		.NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs),
		.NodeMask = 0
	};
	Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void DXExt::CreateShaderBlob_VsPs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".vs.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
#ifdef USE_SHADER_BLOB_PART
	SetBlobPart(ShaderBlobs.back());
	GetBlobPart(COM_PTR_GET(ShaderBlobs.back()));
#endif
	StripShader(ShaderBlobs.back());

	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ps.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
#ifdef USE_SHADER_BLOB_PART
	SetBlobPart(ShaderBlobs.back());
	GetBlobPart(COM_PTR_GET(ShaderBlobs.back()));
#endif
	StripShader(ShaderBlobs.back());
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".vs.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ps.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ds.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".hs.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".gs.cso")) << std::endl;
	ProcessShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
}
void DXExt::CreateShaderBlob_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".cs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
}

void DXExt::CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
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
	const std::array SBCs = {
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[0]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[1]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[1]->GetBufferSize() }),
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	PipelineStates.emplace_back(COM_PTR<ID3D12PipelineState>());
	std::vector<std::thread> Threads;
	//!< メンバ関数をスレッドで使用したい場合は、以下のようにthisを引数に取る形式を使用すればよい
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
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
	const std::array SBCs = {
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[0]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[1]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[1]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[2]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[2]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[3]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[3]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ .pShaderBytecode = ShaderBlobs[4]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[4]->GetBufferSize() }),
	};
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	PipelineStates.emplace_back(COM_PTR<ID3D12PipelineState>());
	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.emplace_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RTBDs, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}
