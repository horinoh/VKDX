//!< DXではレイトレーシング系の各種シェーダは1ファイルにまとめるしかない？

struct Payload
{
    float3 Color;
};

RaytracingAccelerationStructure TLAS : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);
#if 0
ByteAddressBuffer Vertices : register(t1);
ByteAddressBuffer Indices : register(t2);
#endif
//struct TRANSFORM
//{
//    float4x4 InvView;
//    float4x4 InvViewProjection;
//};
//ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    const float2 t = (float2) DispatchRaysIndex().xy / DispatchRaysDimensions().xy;
    
    //!< ペイロードを初期化 (Initialize payload)
    Payload Pay;
    Pay.Color = float3(0.0f, 0.0f, 0.0f);
    
    //!< レイ (Ray)
    RayDesc Ray;
    Ray.TMin = 0.0f;
    Ray.TMax = 10000.0f;
    //Ray.Origin = mul(Transform.InvView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    //const float4 Screen = mul(Transform.InvViewProjection, float4(-2.0f * t + 1.0f, 0.0f, 1.0f));
    //Ray.Direction = normalize(Screen.xyz / Screen.w - Ray.Origin);
    Ray.Origin = float3(0.0f, 0.0f, 1.0f);
    Ray.Direction = normalize(float3(t * 2.0f - 1.0f, 0.0f) - Ray.Origin);
    
    //!< トレーシング (Tracing)
    TraceRay(TLAS, RAY_FLAG_NONE/*RAY_FLAG_CULL_BACK_FACING_TRIANGLES*/, 0xff, 0, 1, 0, Ray, Pay);
    
    //!< 結果をレンダーターゲットへ (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Pay.Color, 1.0f);
}
[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //!< v0, v1, v2 の３頂点の場合、BuiltInTriangleIntersectionAttributes.barycentris.xy はそれぞれ v1, v2 のウエイト
    //!< v = v0 + BuiltInTriangleIntersectionAttributes.barycentrics.x * (v1 - v0) + BuiltInTriangleIntersectionAttributes.barycentrics.y * (v2 - v0)
    const float3 BaryCentrics = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
    
#if 0
    {
        static const uint IndexStride = 3 * 4;
        static const uint VertexStride = (3/*+ 4*/) * 4;
        
        const uint3 VertexIndices = Indices.Load3(PrimitiveIndex() * IndexStride);
        float3 Vertex = float3(0, 0, 0);
        //float4 Color = float4(0, 0, 0, 0);
        [unroll]
        for (uint i = 0; i < 3; ++i) {
            Vertex += asfloat(Vertices.Load3(VertexIndices[i] * VertexStride)) * BaryCentrics[i];
            //Color += asfloat(Vertices.Load4(VertexIndices[i] * VertexStride + 3 * 4)) * BaryCentrics[i];
        }
    }
#endif

    Pay.Color = BaryCentrics;
}
[shader("miss")]
void OnMiss(inout Payload Pay)
{
    Pay.Color = float3(0.529411793f, 0.807843208f, 0.921568692f);
}

//!< ここでは使用しない (Not used here)
//[shader("intersection")]
//[shader("anyhit")]
//[shader("callable")]