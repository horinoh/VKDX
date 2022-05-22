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
    const float3 BaryCentric = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
    Pay.Color = BaryCentric;

    float3 Pos[3], Nrm[3];
    for (int i = 0; i < 3; ++i) {
        const uint Index = IndexBuffer[PrimitiveIndex() * 3 + i];
        Pos[i] = VertexBuffer[Index].Position;
        Nrm[i] = VertexBuffer[Index].Normal;
    }
    const float3 HitPos = Pos[0] * BaryCentric.x + Pos[1] * BaryCentric.y + Pos[2] * BaryCentric.z;
    //const float3 HitNrm = Nrm[0] * BaryCentric.x + Nrm[1] * BaryCentric.y + Nrm[2] * BaryCentric.z;
    const float3 HitNrm = float3(0, 1, 0);// TODO

    const float3 WorldPos = mul(float4(HitPos, 1.0f), ObjectToWorld4x3());
    const float3 WorldNrm = mul(HitNrm, (float3x3)ObjectToWorld4x3());

    //!< Reflection
    {
        Payload Pay1;
        Pay1.Color = float3(0.0f, 0.0f, 0.0f);
        RayDesc Ray;
        Ray.TMin = 0.001f;
        Ray.TMax = 100000.0f;
        Ray.Origin = WorldPos;
        Ray.Direction = reflect(WorldRayDirection(), WorldNrm);
        TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay1);
        Pay.Color = Pay1.Color;
    }

    //!< Refraction
    {
        const float Index = 1.3334f; //!<y‹üÜ—¦z…
        //const float Index = 1.5443f; //!<y‹üÜ—¦z…»
        //const float Index = 2.417f; //!<y‹üÜ—¦zƒ_ƒCƒ„ƒ‚ƒ“ƒh
        const float3 N = normalize(WorldNrm);
        const float3 L = normalize(WorldRayDirection());
        const float NL = dot(N, L);
        const float3 Refracted = refract(L, N, 1.0f / Index); //!< In
        //const float3 Refracted = refract(L, -N, Index); //!< Out

        Payload Pay2;
        Pay2.Color = float3(0.0f, 0.0f, 0.0f);
        RayDesc Ray;
        Ray.TMin = 0.001f;
        Ray.TMax = 100000.0f;
        Ray.Origin = WorldPos;
        Ray.Direction = Refracted;
        TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Pay2);
        Pay.Color = Pay2.Color;
    }
}
