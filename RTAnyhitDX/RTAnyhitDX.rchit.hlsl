struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

float3 ToCartesian(const float3 Barycentric, const float3 p0, const float3 p1, const float3 p2)
{
    return Barycentric.x * p0 + Barycentric.y * p1 + Barycentric.z * p2;
}

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }    
#if true    
    Payload.Color = float3(1.0f, 0.0f, 0.0f);
#else
    const float3 Barycentric = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
    
    const float3 p0 = float3(-1.0f, 1.0f, 0.0f);
    const float3 p1 = float3(-1.0f, -1.0f, 0.0f);
    const float3 p2 = float3(1.0f, 1.0f, 0.0f);
    
    const float3 Cartesian = ToCartesian(Barycentric, p0, p1, p2);
    
    Payload.Color = float3(Cartesian.x, -Cartesian.y, 0.0f) * 0.5f + 0.5f;
    Payload.Color = float3(Cartesian.x, -Cartesian.y, 0.0f);
#endif
}
