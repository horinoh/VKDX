struct Payload
{
    float3 Color;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
struct VertexPN 
{ 
    float3 Position; 
    float3 Normal;
};
StructuredBuffer<VertexPN> VertexBuffer : register(t0, space1);
StructuredBuffer<uint> IndexBuffer : register(t1, space1);

[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    float3 Pos[3], Nrm[3];
    for (int i = 0; i < 3; ++i) {
        Pos[i] = VertexBuffer[IndexBuffer[PrimitiveIndex() * 3 + i]].Position;
        Nrm[i] = VertexBuffer[IndexBuffer[PrimitiveIndex() * 3 + i]].Normal;
    }
    const float3 HitPos = Pos[0] * (1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y) + Pos[1] * BITIA.barycentrics.x + Pos[2] * BITIA.barycentrics.y;
    const float3 HitNrm = Nrm[0] * (1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y) + Nrm[1] * BITIA.barycentrics.x + Nrm[2] * BITIA.barycentrics.y;

    Payload Pay1;
    Pay1.Color = float3(0.0f, 0.0f, 0.0f);
    RayDesc Ray;
    Ray.TMin = 0.001f;
    Ray.TMax = 100000.0f;
    Ray.Origin = mul(float4(HitPos, 1.0f), ObjectToWorld4x3());
    Ray.Direction = reflect(WorldRayDirection(), mul(HitNrm, (float3x3)ObjectToWorld4x3()));
    TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay1);
    Pay.Color = Pay1.Color;

    Pay.Color = HitNrm * 0.5f + 0.5f;
}
