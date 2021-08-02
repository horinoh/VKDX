//!< DXではレイトレーシング系の各種シェーダは1ファイルにまとめるしかない？

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
 
    //!< UV
    const float2 PixelCenter = (float2)DispatchRaysIndex().xy + 0.5f;
    const float2 UV = PixelCenter / DispatchRaysDimensions().xy  * 2.0f - 1.0f;

    //!< レイ (Ray)
    RayDesc Ray;
    Ray.TMin = 0.0f;
    Ray.TMax = 10000.0f;
#if 0
    Ray.Origin = mul(InvView, float4(0.0f, 0.0f, 0.0f, 1)).xyz;
    const float4 Target4 = mul(InvViewProj, float4(UV, 0.0f, 1.0f));
    const float3 Target = Target4.xyz / Target4.w;
#else
    Ray.Origin = float3(0.0f, 0.0f, 0.0f);
    const float3 Target = float3(UV, -6.0f);
#endif
    Ray.Direction = normalize(Target - Ray.Origin);
    //!< トレーシング (Tracing)
    TraceRay(TLAS, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 1, 0, Ray, Pay);
    //TraceRay(TLAS, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 1, 0, Ray, Pay);
    //TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay);

    //!< 結果をレンダーターゲットへ (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Pay.Color, 1.0f);
    //!< UV デバッグ描画
    RenderTarget[DispatchRaysIndex().xy] = float4(abs(UV), 0.0f, 1.0f);
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