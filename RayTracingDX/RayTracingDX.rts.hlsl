//!< DXではレイトレーシング系の各種シェーダは1ファイルにまとめるしかない？

//struct RayDesc
//{
//    float3 Origin;
//    float TMin;
//    float3 Direction;
//    float TMax;
//};

struct Payload
{
    float4 Color;
};

RaytracingAccelerationStructure Scene : register(t0);
ByteAddressBuffer Vertices : register(t1);
ByteAddressBuffer Indices : register(t2);
RWTexture2D<float4> RenderTarget : register(u0);
//ConstantBuffer<XXX> XXX : register(b0);

[shader("raygeneration")]
void OnRayGeneration()
{
    const float2 t = (float2)DispatchRaysIndex().xy / DispatchRaysDimensions().xy;
    
    //!< ペイロードを初期化 (Initialize payload)
    Payload Pay;
    Pay.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    //!< レイ
    RayDesc Ray;
    Ray.TMin = 0.0f;
    Ray.TMax = 10000.0f;
    //Ray.Origin = mul(InvView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    //const float4 Screen = mul(InvViewProj, float4(-2.0f * t + 1.0f, 0.0f, 1.0f));
    //Ray.Direction = normalize(Screen.xyz / Screen.w - Ray.Origin.xyz);
    
    //!< トレーシング
    TraceRay(Scene, RAY_FLAG_NONE /*RAY_FLAG_CULL_BACK_FACING_TRIANGLES*/, ~0, 0, 1, 0, Ray, Pay);
    
    //!< 結果をレンダーターゲットへ
    RenderTarget[DispatchRaysIndex().xy] = Pay.Color;
}
[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //!< a0, a1, a2 の３頂点の場合、barycentris.xy はそれぞれ a1, a2 のウエイト
    //!< a = a0 + barycentrics.x * (a1 - a0) + barycentrics.y * (a2 - a0)
    const float3 BaryCentrics = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
    
    Pay.Color.rgb = float3(0.0f, 1.0f, 0.0f);
}
[shader("miss")]
void OnMiss(inout Payload Pay)
{
    Pay.Color.rgb = float3(0.529411793f, 0.807843208f, 0.921568692f);
}

//!< ここでは使用しない (Not used here)
//[shader("intersection")]
//[shader("anyhit")]
//[shader("callable")]