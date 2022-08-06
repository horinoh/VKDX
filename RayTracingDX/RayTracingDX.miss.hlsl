struct Payload
{
    float3 Color;
};

SamplerState Sampler : register(s0, space0);
TextureCube CubeMap : register(t1, space0);

[shader("miss")]
void OnMiss(inout Payload Pay)
{
    //!< 自動的にミップマップを決定できないので SampleLevel() で明示的にミップレベルを指定する
    Pay.Color = CubeMap.SampleLevel(Sampler, WorldRayDirection(), 0.0f).rgb;
}
