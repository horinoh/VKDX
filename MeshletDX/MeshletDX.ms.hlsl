struct PAYLOAD_IN
{
    uint InstanceID;
    uint MeshletDimension;
    uint MeshletIDs[32];
};

struct VERT_OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

#define N 5
#define NN (N * N)
#define N1 (N - 1)
#define N1N1 (N1 * N1)

static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f) };

static const float PI = 4.0f * atan(1.0f);
float2 GetUV_Torus(const float2 uv)
{
    return frac(uv) * 2.0f * PI;
}
float3 GetPosition_Torus(const float2 uv)
{
    const float2 UV = GetUV_Torus(uv);
    const float2 R = float2(0.5f, 1.0f);
    return float3((R.y + R.x * cos(UV.y)) * cos(UV.x), (R.y + R.x * cos(UV.y)) * sin(UV.x), R.x * sin(UV.y));
}

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

    const float2 MeshletScale = float2(1.0f, 1.0f) / Payload.MeshletDimension;
    const float2 MeshletOffset = float2((float)(MeshletID % Payload.MeshletDimension), (float)(MeshletID / Payload.MeshletDimension)) * MeshletScale;

    const float2 UV = float2((float)(GroupThreadID % N) / N1, 1.0f - (float)(GroupThreadID / N) / N1);
#if 1
    const float3 InstanceOffset = float3((float)(Payload.InstanceID % 4) / 3 - 0.5f, (float)(Payload.InstanceID / 4) / 3 - 0.5f, 0.0f);
    Vertices[GroupThreadID].Position = float4(GetPosition_Torus(UV * MeshletScale + MeshletOffset) * 0.1f + InstanceOffset, 1.0f);
#else
    Vertices[GroupThreadID].Position = float4(UV * MeshletScale + MeshletOffset, 0.0f, 1.0f);
#endif

    Vertices[GroupThreadID].Color = Colors[MeshletID % 8];
}
