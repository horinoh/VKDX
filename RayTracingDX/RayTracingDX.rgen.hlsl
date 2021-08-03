struct Payload
{
    float3 Color;
};

RaytracingAccelerationStructure TLAS : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);

[shader("raygeneration")]
void OnRayGeneration()
{
    //!< ペイロードを初期化 (Initialize payload)
    Payload Pay;
    Pay.Color = float3(0.0f, 0.0f, 0.0f);
 
    //!< +0.5f はピクセルの中心にするため
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;

    //!< レイ (Ray)
    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float エラー対策 非 0.0f の小さな値にする
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UV.x, -UV.y, 1.0f);
    Ray.Direction = float3(0.0f, 0.0f, -1.0f);

    //!< トレーシング (Tracing)
    TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay);

    //!< 結果をレンダーターゲットへ (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Pay.Color, 1.0f);
    //RenderTarget[DispatchRaysIndex().xy] = float4(UV.x, -UV.y, 0.0f, 1.0f);
}
