struct IN
{
	float4 Position : SV_POSITION;
	float3 Position3D : TEXCOORD2;
};

SamplerState Sampler : register(s0, space0);
TextureCube CubeMap : register(t0, space0);

struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	Out.Color = CubeMap.Sample(Sampler, In.Position3D.xyz);

	return Out;
}

