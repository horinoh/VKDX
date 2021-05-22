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
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, out indices uint3 Indices[2 * N1N1], out vertices VERT_OUT Vertices[NN])
{
    /*
    V = N * N ... 4, 9, 16
    P = 2(N - 1)(N - 1) ... 2, 4, 8, 18,
    +-+-+ 0, 1, ..., N-1
    | | |
    +-+-+ N, N+1, ..., N+(N-1)
    | | |
    +-+-+ 2N, 2N+1, ..., 2N+(N-1)
    | | |
    +-+-+ NN, NN+1, ..., NN+(N-1)
    */
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
    
    const uint x = GroupThreadID % N;
    const uint y = GroupThreadID / N;
    Vertices[GroupThreadID].Position = float4(float3((float)x / N1, 1.0f - (float)y / N1, 0.0f) - float3(0.5f, 0.5f, 0.0f), 1.0f);
    Vertices[GroupThreadID].Color = float3(GroupThreadID % 2, (GroupThreadID / N) % 2, 0.0f);

    /*
    6 = 4 + 2, 15 = 9 + 2 * 3, 28 = 16 + 3 * 4, N*N + (N-1)*N
    V = (N + 1) * (2N + 1) ... 6, 15, 28
    P = 2 * 2(N - 1)(N - 1) ... 4, 16, 36, 

    0 1 2 N
    +-+-+
    | | |   
    +-+-+      
    | | |
    +-+-+
    | | |   
    +-+-+      
    | | |
    +-+-+
    12 13 14
    */
}
