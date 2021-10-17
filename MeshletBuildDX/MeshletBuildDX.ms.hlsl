#define MESHLET_COUNT 32

struct PAYLOAD_IN
{
    uint MeshletIDs[MESHLET_COUNT];
};

struct VERT_IN
{
    float3 Position;
};
struct MESHLET
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};
StructuredBuffer<VERT_IN>   InVertices      : register(t0);
ByteAddressBuffer           VertexIndices   : register(t1);
StructuredBuffer<MESHLET>   Meshlets        : register(t2);
StructuredBuffer<uint>      Triangles       : register(t3);

uint3 Unpack(const uint tri)
{
    //!< uint32_t の 30bit を使用して i0, i1, i2 それぞれ 10bit
    return uint3(tri & 0x3ff, (tri >> 10) & 0x3ff, (tri >> 20) & 0x3ff);
}
uint GetVertexIndex32(const uint i) 
{
    return VertexIndices.Load(i << 2); //!< Load() の引数は byte 指定なので 4 を掛けている 
}
uint GetVertexIndex16(const uint i) 
{
    //!< 16bit の場合 uint に インデックスが 2 つ含まれるので i を 2 で割り、i が奇数なら 16bit シフトして取得している
    return (GetVertexIndex32(i >> 1) >> (16 * (i & 0x1))) & 0xffff;
}

struct VERT_OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f) };

static const float4x4 VP = transpose(float4x4(1.93643105f, 0.0f, 0.0f, 0.0f,
    0.0f, 3.89474249f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.00010002f, -1.0f,
    0.0f, 0.0f, 2.99029899f, 3.0f));

[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, in payload PAYLOAD_IN Payload, out indices uint3 OutIndices[64], out vertices VERT_OUT OutVertices[126])
{
    const uint MeshletID = Payload.MeshletIDs[GroupID];
    const MESHLET ML = Meshlets[MeshletID];

    SetMeshOutputCounts(ML.VertCount, ML.PrimCount);

    OutIndices[GroupThreadID] = Unpack(Triangles[ML.PrimOffset + GroupThreadID]);
    
    //OutVertices[GroupThreadID].Position = mul(VP, float4(InVertices[GetVertexIndex32(ML.VertOffset + GroupThreadID)].Position, 1.0f));
    OutVertices[GroupThreadID].Position = mul(VP, float4(InVertices[ML.VertOffset + GroupThreadID].Position, 1.0f));

    OutVertices[GroupThreadID].Color = Colors[MeshletID % 8];
}
