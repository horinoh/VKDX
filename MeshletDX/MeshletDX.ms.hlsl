struct PAYLOAD_IN
{
    uint MeshletIDs[32];
};

struct VERT_OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

#define N 4
#define NN (N * N)
#define N1 (N - 1)
#define N1N1 (N1 * N1)

static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f) };

[numthreads(NN, 1, 1)]
[outputtopology("triangle")]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, in payload PAYLOAD_IN Payload, out indices uint3 Indices[2 * N1N1], out vertices VERT_OUT Vertices[NN])
{
    SetMeshOutputCounts(NN, 2 * N1N1);

    uint PrimCount = 0;
    for (uint i = 0; i < N1; ++i)
    {
        for (uint j = 0; j < N1; ++j)
        {
            const uint LT = i * N + j;
            const uint RT = LT + 1;
            const uint LB = LT + N;
            const uint RB = LB + 1;
            Indices[PrimCount++] = uint3(LT, LB, RT);
            Indices[PrimCount++] = uint3(LB, RB, RT);
        }
    }

    const uint MeshletID = Payload.MeshletIDs[GroupID];

    const uint m = 4;
#if 1
    const float2 Scale = float2(1.0f, 1.0f) / m;
    const float2 Offset = float2((float)(MeshletID % m), (float)(MeshletID / m)) * Scale;
#else
    const float2 Scale = float2(2.0f, 2.0f) / m;
    const float2 Offset = float2((float)(MeshletID % m) - 2.0f, 1.0f - (float)(MeshletID / m)) * Scale;
#endif

    const uint2 Coord = uint2(GroupThreadID % N, GroupThreadID / N);
    Vertices[GroupThreadID].Position = float4(float2((float)Coord.x / N1, 1.0f - (float)Coord.y / N1) * Scale + Offset, 0.0f, 1.0f);
    Vertices[GroupThreadID].Color = Colors[MeshletID % 8];
}
