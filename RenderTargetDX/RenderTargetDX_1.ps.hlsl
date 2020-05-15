struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);

float4 main(IN In) : SV_TARGET
{
#if 0
	const float2 UV = In.Texcoord;
#elif 0
	//!< ラスター
	const float PI = 4.0f * atan(1.0f);
	const float2 UV = In.Texcoord + float2(0.05f * cos(In.Texcoord.y * PI * 10.0f), 0.0f);
#else
	//!< モザイク
	const float2 Resolution = float2(800.0f, 600.0f);
	const float Block = 10.0f;
	const float2 UV = floor(In.Texcoord * Resolution / Block) * Block / Resolution;
#endif
	return Texture.Sample(Sampler, UV);
}
