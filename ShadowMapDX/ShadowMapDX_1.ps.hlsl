struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; float4x4 InverseViewProjection; };

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);	//!< [“x(Depth)

float4 main(IN In) : SV_TARGET
{
	return float4(Texture.Sample(Sampler, In.Texcoord).rrr, 1.0f);
}
