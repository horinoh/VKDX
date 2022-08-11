struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
struct VertexPN 
{ 
    float3 Position; 
    float3 Normal;
};
StructuredBuffer<VertexPN> VB : register(t0, space1);
StructuredBuffer<uint3> IB : register(t1, space1);
//Texture2D<float4> Diffuse : register(t2, space1); Diffuse.SampleLevel(Sampler, UV, 0.0f);

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 31)
    {
        Payload.Color = float3(0.0f, 0.0f, 0.0f);
        return;
    }
    
    const float3 BaryCentric = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    //!< ƒvƒŠƒ~ƒeƒBƒuƒCƒ“ƒfƒbƒNƒX : GLSL gl_PrimitiveID ‘Š“–
    const uint3 Index = IB[PrimitiveIndex()];
    VertexPN Hit;
    Hit.Position = VB[Index.x].Position * BaryCentric.x + VB[Index.y].Position * BaryCentric.y + VB[Index.z].Position * BaryCentric.z;
    Hit.Normal = normalize(VB[Index.x].Normal * BaryCentric.x + VB[Index.y].Normal * BaryCentric.y + VB[Index.z].Normal * BaryCentric.z);
    //Hit.Normal = float3(0, 1, 0);
    
    //const float3 WorldPos = mul(float4(Hit.Position, 1.0f), ObjectToWorld4x3());
    //const float3 WorldNrm = mul(Hit.Normal, (float3x3) ObjectToWorld4x3());
    const float3 WorldPos = mul(ObjectToWorld3x4(), float4(Hit.Position, 1.0f));
    const float3 WorldNrm = mul((float3x3) ObjectToWorld3x4(), Hit.Normal);

    switch(InstanceID()) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
    }
    
    //!< Reflection
    {
        PAYLOAD Reflect;
        Reflect.Color = float3(0.0f, 0.0f, 0.0f);
        Reflect.Recursive = Payload.Recursive;
        RayDesc Ray;
        Ray.TMin = 0.001f;
        Ray.TMax = 100000.0f;
        Ray.Origin = WorldPos;
        Ray.Direction = reflect(WorldRayDirection(), WorldNrm);
        TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Reflect);
        Payload.Color = Reflect.Color;
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

        PAYLOAD Refract;
        Refract.Color = float3(0.0f, 0.0f, 0.0f);
        Refract.Recursive = Payload.Recursive;
        RayDesc Ray;
        Ray.TMin = 0.001f;
        Ray.TMax = 100000.0f;
        Ray.Origin = WorldPos;
        Ray.Direction = Refracted;
        TraceRay(TLAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, Ray, Refract);
        Payload.Color = Refract.Color;
    }
    
    Payload.Color = Hit.Normal * 0.5f + 0.5f;
}
