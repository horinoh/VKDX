struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }    
    const float2 UV = ((float2) DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy;
    Payload.Color = float3(UV, 0.0f);
}
