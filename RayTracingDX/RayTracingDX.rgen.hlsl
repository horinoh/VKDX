struct Payload
{
    float3 Color;
};

RaytracingAccelerationStructure TLAS : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);

struct Transform {
    float4x4 Projection;
    float4x4 View;
    float4x4 InvProjection;
    float4x4 InvView;
};
ConstantBuffer<Transform> CB : register(b0);

[shader("raygeneration")]
void OnRayGeneration()
{
    Payload Pay;
    Pay.Color = float3(0.0f, 0.0f, 0.0f);
 
    //!< +0.5f はピクセルの中心にするため
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float エラー対策 非 0.0f の小さな値にする
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UpY, -1.0f);
    Ray.Direction = float3(0.0f, 0.0f, 1.0f);

    //TraceRay(TLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 1, 0, Ray, Pay);
    TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay);

    RenderTarget[DispatchRaysIndex().xy] = float4(Pay.Color, 1.0f);
}
