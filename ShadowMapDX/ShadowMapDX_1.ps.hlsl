struct IN
{
	float4 Position : SV_POSITION;
	float4 Texcoord : TEXCOORD0;
};

//!< SamplerComparisonState を使用するには、サンプラ作成時に比較方法(D3D12_FILTER_COMPARISON_..., D3D12_COMPARISON_FUNC_...)を指定すること
SamplerComparisonState SamplerComp : register(s0, space0);
Texture2D Texture : register(t0, space0);

float4 main(IN In) : SV_TARGET
{
	In.Texcoord.xyz /= In.Texcoord.w;
#if 1
	const float ShadowFactor = Texture.SampleCmpLevelZero(SamplerComp, In.Texcoord.xy, In.Texcoord.z);
#else
	const float ShadowFactor = (Texture.SampleCmpLevelZero(SamplerComp, In.Texcoord.xy, In.Texcoord.z, int2(-1, -1)) + Texture.SampleCmpLevelZero(SamplerComp, In.Texcoord.xy, In.Texcoord.z, int2( 1, -1)) + Texture.SampleCmpLevelZero(SamplerComp, In.Texcoord.xy, In.Texcoord.z, int2( 1,  1)) + Texture.SampleCmpLevelZero(SamplerComp, In.Texcoord.xy, In.Texcoord.z, int2(-1,  1))) * 0.25f;
#endif
	return float4(ShadowFactor, ShadowFactor, ShadowFactor, 1.0f);
}
