struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

Texture2DArray<float4> LeapMap : register(t0, space0);
Texture2DArray<float4> DistortionMap : register(t1, space0);
SamplerState Sampler : register(s0, space0);

float4 main(IN In) : SV_TARGET
{
    const float ArrayIndex = 1.0f - step(In.Texcoord.x, 0.5f);
    const float2 UV = float2(frac(In.Texcoord.x * 2.0f), In.Texcoord.y);
#if 1
    const float2 DistortionIndex = DistortionMap.Sample(Sampler, float3(UV, ArrayIndex)).xy;
    clip(DistortionIndex);
    clip(1.0f - DistortionIndex);
    return float4(LeapMap.Sample(Sampler, float3(DistortionIndex, ArrayIndex)).rrr, 1.0f);
#else
    return LeapMap.Sample(Sampler, float3(UV, ArrayIndex));
    //return DistortionMap.Sample(Sampler, float3(UV, ArrayIndex));
#endif
}
