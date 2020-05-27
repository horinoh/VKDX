struct IN
{
	float4 Position : SV_POSITION;
	//float2 Texcoord : TEXCOORD0;
	float4 Texcoord : TEXCOORD0;
};

//SamplerState Sampler : register(s0, space0);
SamplerComparisonState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);	//!< [“x(Depth)

float4 main(IN In) : SV_TARGET
{
	//return float4(Texture.Sample(Sampler, In.Texcoord).rrr, 1.0f);
#if 0
	const float ShadowFactor = Texture.SampleCmpLevelZero(Sampler, In.Texcoord.xy, In.Texcoord.z);
#else
	const float ShadowFactor = (Texture.SampleCmpLevelZero(Sampler, In.Texcoord.xy, In.Texcoord.z, int2(-1, -1)) + Texture.SampleCmpLevelZero(Sampler, In.Texcoord.xy, In.Texcoord.z, int2( 1, -1)) + Texture.SampleCmpLevelZero(Sampler, In.Texcoord.xy, In.Texcoord.z, int2( 1,  1)) + Texture.SampleCmpLevelZero(Sampler, In.Texcoord.xy, In.Texcoord.z, int2(-1,  1))) * 0.25f;
#endif
	return float4(ShadowFactor, ShadowFactor, ShadowFactor, 1.0f);
}
