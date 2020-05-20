struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	uint Viewport : SV_ViewportArrayIndex;
};

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);	//!< カラー(Color)
Texture2D Texture1 : register(t1, space0);	//!< 法線(Normal)
Texture2D Texture2 : register(t2, space0);	//!< 深度(Depth)
Texture2D Texture3 : register(t3, space0);	//!< 未定

float4 main(IN In) : SV_TARGET
{
	float4 OutColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	static const float4 Mask[] = {
		float4(1.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 1.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, 0.0f),
		float4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	//!< 分割画面ではビューポート1以降を使用しているのでMask[]の添え字と合わせるために-1 (Devided screen use viewport [1, 4])
	const uint VPIndex = In.Viewport - 1;

	OutColor.rgb += Texture.Sample(Sampler, In.Texcoord).rgb * Mask[VPIndex].xxx;

	OutColor.rgb += Texture1.Sample(Sampler, In.Texcoord).rgb * Mask[VPIndex].yyy;

	OutColor.rgb += Texture2.Sample(Sampler, In.Texcoord).rrr * Mask[VPIndex].zzz;

	OutColor.rgb += Texture3.Sample(Sampler, In.Texcoord).rgb * Mask[VPIndex].www;

	return OutColor;
}
