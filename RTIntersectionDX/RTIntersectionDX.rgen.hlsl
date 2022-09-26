struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    PAYLOAD Payload;
    Payload.Color = float3(0.0f, 0.0f, 0.0f);
    Payload.Recursive = 0;
    
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    RayDesc Ray;
    Ray.TMin = 0.001f; 
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UpY, -1.0f);
    Ray.Direction = float3(0.0f, 0.0f, 1.0f);

    TraceRay(TLAS, 
            RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH  ,
            0xff,
            0, 0,
            0,      
            Ray, 
            Payload);

    RenderTarget[DispatchRaysIndex().xy] = float4(Payload.Color, 1.0f);
}
