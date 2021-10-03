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
StructuredBuffer<MESHLET>   Meshlets        : register(t1);
ByteAddressBuffer           VertexIndices   : register(t2);
StructuredBuffer<uint>      Triangles       : register(t3);
uint3 Unpack(const uint tri)
{
    return uint3(tri & 0x3ff, (tri >> 10) & 0x3ff, (tri >> 20) & 0x3ff);
}
uint GetVertexIndex32(const uint i) 
{
    return VertexIndices.Load(i << 2); //!< Load() ‚Ìˆø”‚Í byte Žw’è‚È‚Ì‚Å 4 ‚ðŠ|‚¯‚Ä‚¢‚é 
}
uint GetVertexIndex16(const uint i) 
{
    return (GetVertexIndex32(i >> 1) >> (16 * (i & 0x1))) & 0xffff;
}

struct VERT_OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f) };

[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, in payload PAYLOAD_IN Payload, out indices uint3 Indices[126], out vertices VERT_OUT Vertices[64])
{
    MESHLET ML = Meshlets[Payload.MeshletIDs[GroupID]];
    
    SetMeshOutputCounts(ML.VertCount, ML.PrimCount);

    Indices[GroupThreadID] = Unpack(Triangles[ML.PrimOffset + GroupThreadID]);
    
    const float4x4 WVP = transpose(float4x4(1.93643105f, 0.0f, 0.0f, 0.0f,
        0.0f, 3.89474249f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.00010002f, -1.0f,
        0.0f, 0.0f, 2.99029899f, 3.0f));
    //Vertices[GroupThreadID].Position = mul(WVP, float4(InVertices[GetVertexIndex32(ML.VertOffset + GroupThreadID)].Position, 1.0f));
    Vertices[GroupThreadID].Position = mul(WVP, float4(InVertices[ML.VertOffset + GroupThreadID].Position, 1.0f));

    Vertices[GroupThreadID].Color = Colors[GroupID % 8];
}
