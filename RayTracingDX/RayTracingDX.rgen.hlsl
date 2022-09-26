struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);
struct Transform {
    float4x4 Projection;
    float4x4 View;
    float4x4 InvProjection;
    float4x4 InvView;
};
ConstantBuffer<Transform> CB : register(b0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    PAYLOAD Payload;
    Payload.Color = float3(0.0f, 0.0f, 0.0f);
    Payload.Recursive = 0;
    
    //!< +0.5f はピクセルの中心にするため
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float エラー対策 非 0.0f の小さな値にする
    Ray.TMax = 100000.0f;
    Ray.Origin = mul(CB.InvView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    const float4 Target = mul(CB.InvProjection, float4(UpY, 1.0f, 1.0f));
#if 1
    Ray.Direction = mul(CB.InvView, float4(normalize(Target.xyz / Target.w), 0.0f)).xyz;
#else
    Ray.Direction = normalize(mul(CB.InvView, float4(Target.xyz, 0.0f)).xyz);
#endif

    //TraceRay(TLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 1, 0, Ray, Payload);
    TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 0, 0, Ray, Payload);

    RenderTarget[DispatchRaysIndex().xy] = float4(Payload.Color, 1.0f);
}
