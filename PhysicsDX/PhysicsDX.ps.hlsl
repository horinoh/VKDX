struct IN
{
	float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Color : SV_TARGET;
};

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;
    //Out.Color = float4(In.Normal * 0.5f + 0.5f, 1.0f);
    Out.Color = float4(In.Texcoord, 0.0f, 1.0f);
	return Out;
}

