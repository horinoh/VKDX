struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
struct PAYLOAD_SHADOW
{
    bool IsOccluded;
};

//struct VertexP
//{
//    float3 Position;
//};
//StructuredBuffer<VertexP> VB : register(t0, space1);
//StructuredBuffer<uint3> IB : register(t1, space1);

float3 ToBaryCentric(const float2 Attr)
{
    return float3(1.0f - Attr.x - Attr.y, Attr.x, Attr.y);
}
float3 ToCartesian(const float3 A, const float3 B, const float3 C, const float3 BaryCentric)
{
    return BaryCentric.x * A + BaryCentric.y * B + BaryCentric.z * C;
}
float3 ToCartesian(const float3 V[3], const float3 BaryCentric)
{
    return ToCartesian(V[0], V[1], V[2], BaryCentric);
}

//!< 引数をとるとダメ
bool ShadowRay(const float3 Origin, const float3 Direction)
{    
    RayDesc Ray;
    Ray.TMin = 0.001f;
    Ray.TMax = 100000.0f;
    Ray.Origin = Origin;
    Ray.Direction = Direction;
    
    PAYLOAD_SHADOW PayloadShadow;
    PayloadShadow.IsOccluded = false;
    
    TraceRay(TLAS,
            RAY_FLAG_FORCE_OPAQUE /*| RAY_FLAG_CULL_BACK_FACING_TRIANGLES*/ | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
            0xff,
            0,
            0,
            1, //!< 1 番のミスシェーダ
            Ray,
            PayloadShadow);
    
    return PayloadShadow.IsOccluded;
}

const float3 LightPos = float3(0.0f, 100.0f, 0.0f);

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }
    Payload.Color = ToBaryCentric(BITIA.barycentrics);
    
    const float3 Pos = ToCartesian(float3(0, -1, 0), float3(0, -1, 0), float3(0, -1, 0), ToBaryCentric(BITIA.barycentrics));
    if (ShadowRay(Pos, normalize(LightPos - Pos)))
    {
        Payload.Color *= 0.5f;
    }
}
