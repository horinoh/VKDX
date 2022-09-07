struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

struct VertexNT
{
    float3 Normal;
    float2 Texcoord;
};

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in VertexNT Attr)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }
    Payload.Color = float3(1.0f, 1.0f, 1.0f);
}
