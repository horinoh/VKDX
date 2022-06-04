struct Payload
{
    float3 Color;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    //!< ペイロードを初期化 (Initialize payload)
    Payload Pay;
    Pay.Color = float3(0.0f, 0.0f, 0.0f);
 
    //!< +0.5f はピクセルの中心にするため
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    //!< レイ (Ray)
    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float エラー対策 非 0.0f の小さな値にする
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UpY, -1.0f);
    Ray.Direction = float3(0.0f, 0.0f, 1.0f);

    //!< レイトレーシング (RayTracing)
    //!< フラグ
    //!<    RAY_FLAG_NONE,
    //!<    RAY_FLAG_FORCE_OPAQUE, RAY_FLAG_FORCE_NON_OPAQUE,
    //!<    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
    //!<    RAY_FLAG_CULL_BACK_FACING_TRIANGLES, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES, RAY_FLAG_CULL_OPAQUE, RAY_FLAG_CULL_NON_OPAQUE = 0x80,
    //!<    RAY_FLAG_SKIP_TRIANGLES, RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES,
    TraceRay(TLAS, 
            RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
            0xff,
            0, 1,   //!< RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex
            0,      //!< MissShaderIndex
            Ray, 
            Pay);

    //!< 結果をレンダーターゲットへ (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Pay.Color, 1.0f);
    //RenderTarget[DispatchRaysIndex().xy] = float4(UpY, 0.0f, 1.0f);
}
