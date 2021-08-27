SamplerState Sampler : register(s0, space0);
TextureCube CubeMap : register(t1, space0);

struct Payload
{
    float3 Color;
};

[shader("miss")]
void OnMiss(inout Payload Pay)
{
    Pay.Color = CubeMap.SampleLevel(Sampler, WorldRayDirection(), 0.0f).rgb;
}
