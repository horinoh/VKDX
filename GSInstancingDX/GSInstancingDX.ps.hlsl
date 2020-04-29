struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	uint Viewport : SV_ViewportArrayIndex;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
	const float4 Colors[] = {
		float4(1.0f, 0.0f, 0.0f, 1.0f),
		float4(0.0f, 1.0f, 0.0f, 1.0f),
		float4(0.0f, 0.0f, 1.0f, 1.0f),
		float4(1.0f, 1.0f, 0.0f, 1.0f),
	};
	Out.Color = Colors[In.Viewport];
	return Out;
}

