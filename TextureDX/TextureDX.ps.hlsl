struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

Texture2D Texture : register(t0, space0);
SamplerState Sampler : register(s0, space0);

float4 main(IN In) : SV_TARGET
{
	//return float4(In.Texcoord, 0.0f, 1.0f);
	return Texture.Sample(Sampler, In.Texcoord);
}
