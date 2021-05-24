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
    
    const float3 Offset = float3(GroupID, GroupID, 0.0f) / 15.0f - float3(1.0f, 1.0f, 0.0f);
    const uint x = GroupThreadID % N;
    const uint y = GroupThreadID / N;
    Vertices[GroupThreadID].Position = float4(float3((float)x / N1, 1.0f - (float)y / N1, 0.0f) + Offset, 1.0f);
    Vertices[GroupThreadID].Color = float3(GroupThreadID % 2, (GroupThreadID / N) % 2, 0.0f);
}
