//!< スレッド
//!<    uint3 DispatchRaysIndex(), DispatchRaysDimemsions()

//!< レイ
//!<    uint RayFlags()
//!<    float3 WorldRayOrigin(), WorldRayDirection(), ObjectRayOrigin(), ObjectRayDirection()
//!<    float RayTMin(), RayTCurrent()

//!< インデックス
//!<    uint InstanceID()(D3D12_RAYTRACING_INSTANCE_DESC.InstanceID), InstanceIndex(), GeometryIndex(), PrimitiveIndex()

//!< オブジェクト <-> ワールド
//!<    float4x3 ObjectToWorld4x3()
//!<    float3x4 ObjectToWorld3x4()
//!<    float4x3 WorldToObject4x3()
//!<    float3x4 WorldToObject3x4()

//!< ヒット
//!<    uint HitKind()(HIT_KIND_TRIANGLE_FRONT_FACE, HIT_KIND_TRIANGLE_BACK_FACE)
    
struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    //!< +0.5f はピクセルの中心にするため
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    //!< レイ (Ray)
    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float エラー対策 非 0.0f の小さな値にする
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UpY, -1.0f);
    Ray.Direction = float3(0.0f, 0.0f, 1.0f);

    //!< ペイロードを初期化 (Initialize payload)
    PAYLOAD Payload;
    Payload.Color = float3(0.0f, 0.0f, 0.0f);
    Payload.Recursive = 0;
    
    //!< レイトレーシング (RayTracing)
    //!< フラグ
    //!<    RAY_FLAG_NONE,
    //!<    RAY_FLAG_FORCE_OPAQUE, RAY_FLAG_FORCE_NON_OPAQUE,
    //!<    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
    //!<    RAY_FLAG_CULL_BACK_FACING_TRIANGLES, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES, RAY_FLAG_CULL_OPAQUE, RAY_FLAG_CULL_NON_OPAQUE = 0x80,
    //!<    RAY_FLAG_SKIP_TRIANGLES, RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES ... 要 DXR1.1

    //!< InstanceContributionToHitGroupIndex    : TLAS の D3D12_RAYTRACING_INSTANCE_DESC.InstanceContributionToHitGroupIndex
    //!< GeometryContributionToHitGroupIndex    : BLAS のジオメトリインデックス (DX ではシェーダから参照不可、VK では可能)
    //!< RecordAddress = RCTHGI + MFGCTHGI * GeometryContributionToHitGroupIndex + InstanceContributionToHitGroupIndex   
    TraceRay(TLAS, 
            RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
            0xff,
            0,      //!< RayContributionToHitGroupIndex (RCTHGI)
            0,      //!< MultiplierForGeometryContributionToHitGroupIndex (MFGCTHGI)
            0,      //!< MissShaderIndex : ここではミスシェーダは 1 つなので 0
            Ray, 
            Payload);

    //!< 結果をレンダーターゲットへ (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Payload.Color, 1.0f);
    //RenderTarget[DispatchRaysIndex().xy] = float4(UpY, 0.0f, 1.0f);
}
