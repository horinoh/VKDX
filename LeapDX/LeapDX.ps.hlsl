struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

Texture2DArray<float4> Texture : register(t0, space0);
Texture2DArray<float4> DistortionMap : register(t1, space0);
SamplerState Sampler : register(s0, space0);

float4 main(IN In) : SV_TARGET
{
    const float2 DistortionIndex = DistortionMap.Sample(Sampler, float3(In.Texcoord, 0.0f)).xy;
    if (DistortionIndex.x < 0.0f || DistortionIndex.x > 1.0f || DistortionIndex.y < 0.0f || DistortionIndex.y > 1.0f)
    {
        return float4(0.5f, 0.5f, 0.5f, 1);
    }
    //return float4(Texture.Sample(Sampler, float3(DistortionIndex, 0.0f)).rrr, 1.0f);
    
    //return Texture.Sample(Sampler, float3(frac(In.Texcoord.x * 2.0f), In.Texcoord.y, 1.0f - step(In.Texcoord.x, 0.5f)));
    return DistortionMap.Sample(Sampler, float3(frac(In.Texcoord.x * 2.0f), In.Texcoord.y, 1.0f - step(In.Texcoord.x, 0.5f)));
}
