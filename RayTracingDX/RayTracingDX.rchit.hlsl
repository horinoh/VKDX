//StructuredBuffer<float3> VertexBuffer : reister(t0, space0)
//StructuredBuffer<uint> IndexBuffer : reister(t1, space0)

struct Payload
{
    float3 Color;
};
//RaytracingAccelerationStructure TLAS : register(t0);

[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    //Payload Pay1;
    //Pay1.Color = float3(0.0f, 0.0f, 0.0f);
    //RayDesc Ray;
    //Ray.TMin = 0.001f;
    //Ray.TMax = 100000.0f;
    //Ray.Origin = mul(float4(CD.Position, 1.0f), ObjectToWorld4x3());
    //Ray.Direction = reflect(WorldRayDirection(), mul(CD.Normal, (float3x3)ObjectToWorld4x3()));
    //TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay1);
    //Pay.Color = Pay1.Color;
}
